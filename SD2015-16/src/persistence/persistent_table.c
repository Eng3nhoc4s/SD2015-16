#include "data.h"
#include "entry.h"
#include "table-private.h"
#include "persistence_manager.h"
#include "persistence_manager-private.h"
#include "persistent_table-private.h"
#include "persistent_table.h"
#include "message-private.h"
#include "message.h"
#include "table.h"
#include "inet.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

struct ptable_t; /* A definir em persistent_table-private.h */

/* Abre o acesso a uma tabela persistente, passando como parâmetros a
 * tabela a ser mantida em memória e o gestor de persistência a ser usado
 * para manter logs e checkpoints. Retorna a tabela persistente criada ou
 * NULL em caso de erro.
 */
struct ptable_t *ptable_open(struct table_t *table, 
                             struct pmanager_t *pmanager){

	//verifica parametros
	if(table == NULL || pmanager == NULL){
		return NULL;
	}

	//aloca memoria para estrutura
	struct ptable_t *ptable = (struct ptable_t*) malloc(sizeof(struct ptable_t));
	if(ptable == NULL){
		return NULL;
	}
	//pmanager
	struct pmanager_t *pm = pmanager;
	if(pm == NULL)
		return NULL;

	//preenche estrutura
	ptable->table = table;
	ptable->pmanager = pm;
	ptable->open = 0;// table is open

	
	int fdSTT = access(processFilename(ptable->pmanager->filename, "stt"), F_OK);
	int fdCHK = access(processFilename(ptable->pmanager->filename, "chk"), F_OK);
	int fdLOG = sizeFile(ptable->pmanager->file);

	printf("**** LOAD TABLE FILE CHECK ****\n");
	if(fdSTT != -1)
		printf("\t> SST EXISTS!\n");
		
	if(fdCHK != -1)
		printf("\t> CHK EXISTS!\n");
		
	if(fdLOG > 0)
		printf("\t> LOG HAS DATA!\n\n");

	printf("**** LOADING TABLE FROM FILES ****\n");
	//Existe STT?
	if(fdSTT != -1){
	printf("**** GET STT ****\n");	
		//existe log?
		if(fdLOG > 0){
			printf("\t> LOG + CHK\n");
			//recupera do chk 
			if(fdCHK != -1){

				int ret = pmanager_restore_table(ptable->pmanager, ptable->table, "chk");
				if(ret < 0) {
					ptable_destroy(ptable);
					return NULL;
				}
			}
			
			//+ recupera do log
			if(pmanager_has_data(ptable->pmanager)) {

				int ret = pmanager_fill_state(ptable->pmanager, ptable->table);
				if(ret < 0) {
					ptable_destroy(ptable);
					return NULL;
				}
			}
		
		//recupera do stt
		} else {
		printf("\t> JUST STT\n");
			int ret = pmanager_restore_table(ptable->pmanager, ptable->table, "stt");
			if(ret < 0) {
				ptable_destroy(ptable);
				return NULL;
			}
		}	
	//Existe chk?
	} else if(fdCHK != -1){
		printf("**** GET CHK ****\n");	
		//existe log?
		if(fdLOG > 0){
			printf("\t> CHK + LOG \n");
			//recupera do chk 
				
				int ret = pmanager_restore_table(ptable->pmanager, ptable->table, "chk");
				if(ret < 0) {
					ptable_destroy(ptable);
					return NULL;
				}
			
			
			//+ recupera do log
			if(pmanager_has_data(ptable->pmanager)) {
				int ret = pmanager_fill_state(ptable->pmanager, ptable->table);
				if(ret < 0) {
					ptable_destroy(ptable);
					return NULL;
				}
			}
			
		
		} else {
			//Recupera do CHK
			printf("\t> JUST CHK \n");
			int ret = pmanager_restore_table(ptable->pmanager, ptable->table, "chk");
			if(ret < 0) {
				ptable_destroy(ptable);
				return NULL;
			}
			
		}	
	//Existe log
	} else if(pmanager_has_data(ptable->pmanager)) {
	
		printf("**** GET LOG ****\n");	
		//recupera do log
		int ret = pmanager_fill_state(ptable->pmanager, ptable->table);
		if(ret < 0) {
			ptable_destroy(ptable);
			return NULL;
		}
	}

	return ptable;
}

/* Fecha o acesso a uma tabela persistente. Todas as operações em table
 * devem falhar após um ptable_close.
 */
void ptable_close(struct ptable_t *ptable){
	if(ptable != NULL){
		ptable->open = 1; // table is closed
	}
}

/* Liberta toda a memória e apaga todos os ficheiros utilizados pela
 * tabela persistente. 
 */
void ptable_destroy(struct ptable_t *ptable){
	if(ptable != NULL){
		table_destroy(ptable->table);
		if(pmanager_destroy_clear(ptable->pmanager)==0){
			pmanager_destroy(ptable->pmanager);
		}
		free(ptable);
	}
}

/* Função para adicionar um par chave valor na tabela.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int ptable_put(struct ptable_t *ptable, char *key, struct data_t *value){
	//verifica se ptable está aberto
	if(ptable->open != 0)
		return -1;

	//faz a operaçao na tabela, em caso de sucesso escreve no log
	if(table_put(ptable->table,key,value)==0){
		
		//inicializa a mensagem
		struct message_t *msg = (struct message_t *) malloc (sizeof(struct message_t));
		if(msg == NULL){
			return -1;
		}
		//preenche os campos de message_t
		msg->opcode = OC_PUT;
		msg->c_type = CT_ENTRY;
		struct entry_t *newEntry = entry_create(key,value); 
		if(newEntry == NULL){
			free(msg);
			return -1;
		} 
		msg->content.entry = newEntry;

		//serializa a mesnagem
		char *msg_buf;
		int sizeBuffer = message_to_buffer(msg, &msg_buf);
		if(sizeBuffer == -1){
			free_message(msg);
			return -1;
		}
		//sucesso da operação de seralização logo vai escrever no file associado ao pmanager
		char * newMsg_bufWithSize = (char *) malloc(sizeBuffer+MYINT);
		if(newMsg_bufWithSize == NULL) {
			free_message(msg);
			free(msg_buf);
			return -1;
		}

		int redeBuffer = htonl(sizeBuffer);
		memcpy(newMsg_bufWithSize, &redeBuffer, MYINT);
		memcpy(newMsg_bufWithSize+MYINT, msg_buf, sizeBuffer);
 		int res = pmanager_log(ptable->pmanager, newMsg_bufWithSize);
		if(res == -1){
			free(newMsg_bufWithSize);
			free_message(msg);
			free(msg_buf);
			return -1;
		}
		
		//VERIFICAR TAMANHO DO LOG, SE FOR MAIOR CRIAR STT (CHAMARA  FC)
		if(sizeFile(ptable->pmanager->file) >= ptable->pmanager->logsize){
	
			pmanager_store_table(ptable->pmanager, ptable->table);

			//ROTATE LOG
			pmanager_rotate_log(ptable->pmanager);
			
			//Remover STT
			res = remove (processFilename(ptable->pmanager->filename, "stt"));
			
			if(res != -1)
				printf("STT REMOVED\n");
			
			//Reabrir o ficheiro de log
			ptable->pmanager->file = fopen(ptable->pmanager->filename, "a+b");
		}
		
		free(newMsg_bufWithSize);
		free_message(msg);
		free(msg_buf);

		return 0;
	}
	return -1;
}
/* Função para substituir na tabela, o valor associado à chave key.
 * Devolve 0 (OK) ou -1 em caso de erros.
 */
int pable_update(struct ptable_t *ptable, char *key, struct data_t *value){
	//verifica se a tabela esta aberta
	if(ptable->open != 0){
		return -1;
	}
	//faz a operaçao na tabela, em caso de sucesso escreve no log
	if(table_update(ptable->table,key,value)==0){
		struct message_t *msg = (struct message_t *) malloc (sizeof(struct message_t));
		if(msg == NULL){
			return -1;
		}
		//prrenche a estrutura message_t
		msg->opcode = OC_UPDATE;
		msg->c_type = CT_ENTRY;
		struct entry_t *newEntry = entry_create(key,value); 
		if(newEntry == NULL){
			free(msg);
			return -1;
		} 
		msg->content.entry = newEntry;

		//serializa a mensagem
		char *msg_buf;
		int sizeBuffer = message_to_buffer(msg, &msg_buf);
		if(sizeBuffer == -1){
			free_message(msg);
			return -1;
		}

		//sucesso da operação de seralização logo vai escrever no file associado ao pmanager
		char * newMsg_bufWithSize = (char *) malloc(sizeBuffer+MYINT);
		if(newMsg_bufWithSize == NULL) {
			free_message(msg);
			free(msg_buf);
			return -1;
		}

		int redeSize = htonl(sizeBuffer);
	
		memcpy(newMsg_bufWithSize, &redeSize, MYINT);
		memcpy(newMsg_bufWithSize+MYINT, msg_buf, sizeBuffer);
 		int res = pmanager_log(ptable->pmanager, newMsg_bufWithSize);
		if(res == -1){
			free(newMsg_bufWithSize);
			free_message(msg);
			free(msg_buf);
			return -1;
		}

		free(newMsg_bufWithSize);
		free_message(msg);
		free(msg_buf);
		return 0;
	}
	return -1;
}

/* Função para obter da tabela o valor associado à chave key.
 * Devolve NULL em caso de erro.
 */
struct data_t *ptable_get(struct ptable_t *ptable, char *key){
	if(ptable->open != 0){
		return NULL;
	}
	return table_get(ptable->table, key);
}

/* Função para remover um par chave valor da tabela, especificado pela
 * chave key.
 * Devolve: 0 (OK) ou -1 em caso de erros
 */
int ptable_del(struct ptable_t *ptable, char *key){
	if(ptable->open != 0){
		return -1;
	}
	if(table_del(ptable->table,key)==0){
		struct message_t *msg = (struct message_t *) malloc (sizeof(struct message_t));
		if(msg == NULL){
			return -1;
		}
		char * dupKey = strdup(key);
		if(dupKey == NULL) {
			free(msg);
			return -1;
		}

		msg->opcode = OC_DEL;
		msg->c_type = CT_KEY;
		msg->content.key = dupKey;

		char *msg_buf;
		int sizeBuffer = message_to_buffer(msg, &msg_buf);
		if(sizeBuffer == -1){
			free_message(msg);
			free(msg_buf);
			return -1;
		}
		
		//sucesso da operação de seralização logo vai escrever no file associado ao pmanager
		char * newMsg_bufWithSize = (char *) malloc(sizeBuffer+MYINT);
		if(newMsg_bufWithSize == NULL) {
			free_message(msg);
			free(msg_buf);
			return -1;
		}

		int redeSize = htonl(sizeBuffer);
		memcpy(newMsg_bufWithSize, &redeSize, MYINT);
		memcpy(newMsg_bufWithSize+MYINT, msg_buf, sizeBuffer);
 		int res = pmanager_log(ptable->pmanager, newMsg_bufWithSize);
		if(res == -1){
			free(newMsg_bufWithSize);
			free_message(msg);
			free(msg_buf);
			return -1;
		}

		free(newMsg_bufWithSize);
		free_message(msg);
		free(msg_buf);
		return 0;	
	}
	return -1;
}

/* Devolve número de elementos na tabela.
 */
int ptable_size(struct ptable_t *ptable){
	if(ptable->open != 0){
		return -1;
	}
	return table_size(ptable->table);
}

/*
Funçao que devolve o numero de operações efectuadas numa tabela
*/
int ptable_num_ops(struct ptable_t *ptable){
	if(ptable->open != 0){
		return -1;
	}
	return table_get_num_change_ops(ptable->table);
}

/* Função para obter o timestamp do valor associado a uma chave.
* Em caso de erro devolve -1. Em caso de chave não encontrada
* devolve 0.
*/
long long ptable_get_ts(struct ptable_t *ptable, char *key) {
	if(ptable->open != 0){
		return -1;
	}
	return table_get_ts(ptable->table, key);
}

/* Devolve um array de char * com a cópia de todas as keys da tabela
 * e um último elemento a NULL.
 */
char **ptable_get_keys(struct ptable_t *ptable){
	if(ptable->open != 0){
		return NULL;
	}
	return table_get_keys(ptable->table);
}
/* Liberta a memória alocada por ptable_get_keys().
 */
void ptable_free_keys(char **keys){
	table_free_keys(keys);
}
