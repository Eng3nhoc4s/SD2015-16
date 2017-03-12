#define _GNU_SOURCE //Para retirar o warning do get_current_dir_name()
#include "table-private.h"
#include "persistence_manager-private.h"
#include "message-private.h"
#include "message.h"
#include "communication.h"
#include "table.h"
#include "inet.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

struct pmanager_t; /* A definir em persistence_manager-private.h */

/* Cria um gestor de persistência que armazena logs em filename+".log".
 * O parâmetro logsize define o tamanho máximo em bytes que o ficheiro de
 * log pode ter.
 * Note que filename pode ser um path completo. Retorna o pmanager criado
 * ou NULL em caso de erro.
 */
struct pmanager_t *pmanager_create(char *filename, int logsize){

	if(filename == NULL || logsize < 10){	
		return NULL;
	}

	struct pmanager_t *pManager = (struct pmanager_t*) malloc (sizeof(struct pmanager_t));
	if(pManager == NULL){
		return NULL;
	}
	
	FILE *fd;

	char * str;
	//Cria o ficheiro, em caso de não ser um path valido cria o ficheiro na diretoria currente. 
	fd = fopen(filename, "a+b");
	if(fd == NULL) {
		if(errno == ENOENT) {

   			char * currDir = get_current_dir_name();

   			if(currDir == NULL) {
   				free(pManager);
   				return NULL;
   			}


   			char* fixedPath= (char *) malloc(strlen(currDir) + strlen("/table.log") + 1 );
   			if(fixedPath == NULL) {
   				free(pManager);
   				free(currDir);
   				return NULL;
   			}

   			strcpy(fixedPath, currDir);
   			strcat(fixedPath, "/table.log");	
   			str = strdup(fixedPath);
   			free(fixedPath);
   			free(currDir);

   			fd = fopen(str, "a+b" );
   			if(fd == NULL) {
   				free(str);
   				free(pManager);
   				return NULL;
   			}

		}
	}

	if(str == NULL);
	str = strdup(filename);

	if(str == NULL){
		free(pManager);
		free(str);
		return NULL;
	}
	pManager->file = fd;
	pManager->logsize = logsize;
	pManager->filename = str;
	
	return pManager;
}

/* Destrói o gestor de persistência pmanager. Retorna 0 se tudo estiver OK
 * ou -1 em caso de erro. Esta função não limpa o ficheiro de log.
 */
int pmanager_destroy(struct pmanager_t *pmanager){
	if (pmanager != NULL){
		if(pmanager->file != NULL)
			fclose(pmanager->file);
		free(pmanager->filename);
		free(pmanager);
		return 0;
	}
	return -1;
}

/* Apaga o ficheiro de log gerido pelo gestor de persistência. 
 * Retorna 0 se tudo estiver OK ou -1 em caso de erro.
 */
int pmanager_destroy_clear(struct pmanager_t *pmanager){
	if(pmanager != NULL){
		fclose(pmanager->file);
		int r = remove(pmanager->filename);
		if(r == 0){
			return 0;
		}else{
			printf("%s\n", "Apagar ficheiro nao resultou");
		}
	}	
	return -1;
}
/* Adiciona uma string msg no fim do ficheiro de log associado a pmanager.
 * Retorna o numero de bytes escritos no log ou -1 em caso de problemas na
 * escrita (e.g., erro no write()), ou no caso em que o tamanho do ficheiro
 * de log após o armazenamento da mensagem seja maior que logsize (neste
 * caso msg não é escrita no log).
 */
int pmanager_log(struct pmanager_t *pmanager, char *msg){
	if(pmanager == NULL || msg == NULL){
		return -1;
	}
	int sizeBuffer;
	memcpy(&sizeBuffer,msg,MYINT);
	sizeBuffer= ntohl(sizeBuffer);

	int nBytesWritten = write_all(fileno(pmanager->file),msg,sizeBuffer+4);

	if(nBytesWritten == 0){
		printf("%s\n", "Ligação terminou, não foi possivel guardar file");
		return -1;
	}
	if(nBytesWritten == -1){
		printf("%s\n", "Erro, não foi possivel guardar file");
		return -1;	
	}
	
	fsync(fileno(pmanager->file));
	
	return nBytesWritten;
}

/* Recupera o estado contido no ficheiro de log, na tabela passada como
 * argumento.
 */
int pmanager_fill_state(struct pmanager_t *pmanager,
                        struct table_t *table){
	if(pmanager == NULL || table == NULL){
		return -1;
	}
	char *msg_buf;
	fclose(pmanager->file); 
	pmanager->file = fopen(pmanager->filename, "rb");
	

	int result = read_all2(fileno(pmanager->file),&msg_buf);

	while(result > 0){

	struct message_t *msg;
	msg = buffer_to_message(msg_buf, result);
	if(msg == NULL){
		return -1;
	}
	switch(msg->opcode){
		case OC_DEL:
			if(table_del(table,msg->content.key)!=0){
				free(msg);
				fclose(pmanager->file);
				return -1;
			}
			break;
		case OC_PUT:
			if(table_put(table,msg->content.entry->key, msg->content.entry->value)!=0){
				free(msg);
				fclose(pmanager->file);
				return -1;
			}
			break;
		case OC_UPDATE:
			if(table_update(table,msg->content.entry->key, msg->content.entry->value)!=0){
				free(msg);
				fclose(pmanager->file);
				return -1;
			}
			break;
		default:
			free(msg);	
			fclose(pmanager->file);
			return -1;
			break;
	}
		free(msg);
		free(msg_buf);
		result = read_all2(fileno(pmanager->file),&msg_buf);
	}
	if(result < 0){
		fclose(pmanager->file);
		return -1;
	}
	fclose(pmanager->file); // fecha o modo de leitura
	fopen(pmanager->filename, "ab"); //abre modo de escrita.
	return 0;
}


/* Cria um ficheiro filename+".stt" com o estado atual da tabela table.
* Retorna o tamanho do ficheiro criado ou -1 em caso de erro.
*/
int pmanager_store_table(struct pmanager_t *pmanager, struct table_t *table){
	struct entry_t ** entries = table_get_entries(table);
	int indexEntries = 0;
	char * msg_buf;
	struct message_t *msg;
	int total= 0;
	//FALTA CRIAR UMA FILENAME+.STT
	FILE * stt = fopen(processFilename(pmanager->filename, "stt"), "ab+");
	
	while (entries[indexEntries] != NULL){
		//criar uma mensagem com esta entrie
		msg = (struct message_t*)malloc(sizeof(struct message_t));
		if(msg == NULL){
			table_free_entries(entries);
			return -1;
		}
		msg->opcode = OC_PUT;
		msg->c_type = CT_ENTRY;
		msg->content.entry = entry_dup(entries[indexEntries]);// PROVAVELMENTE TEM DE SE UTILIZAR O MEMOCPY
		//mandar esta mensagem para ser serializada 
		int msgBuffer = message_to_buffer(msg,&msg_buf);
		if(msgBuffer == -1){
			return -1;
		}
		
		//escrever a msg serializada no file
		int res = write_all2(fileno(stt),msg_buf,msgBuffer);
		if(res == -1){
			fclose(pmanager->file);
			free_message(msg);
			return -1;
		}
	indexEntries++;
	total = total + res;
	}
	fclose(stt);
	table_free_entries(entries);
	free_message(msg);
	return total;
}


/*
* COPIA UM FICHEIRO NA SUA INTEGRA
*/

int copy_file(char *stt, char  *chk)
	{
		FILE  *fd_stt, *fd_chk;
		int  a;

		fd_stt = fopen(stt, "rb");
		fd_chk = fopen(chk, "wb");

		if(fd_stt == NULL)
			return  -1;

		if(fd_chk == NULL)
		{
			fclose(fd_stt);
			return  -1;
		}

		char* buffer [500];
			while (a != EOF) {
	  			a = fread(buffer, 1, 512, fd_stt);
	  			if (a == 0)
	  				break;
	  		fwrite(buffer, 1, a, fd_chk);
  			}
		

		fclose(fd_stt);
		fclose(fd_chk);
		return  0;
	}


/* Limpa o conteúdo do ficheiro ".log" e copia o ficheiro ".stt" para ".chk".
* Retorna 0 se tudo correr bem ou -1 em caso de erro.
*/
int pmanager_rotate_log(struct pmanager_t *pmanager){
	int result = pmanager_destroy_clear(pmanager);
	if(result == -1){
		return -1;
	}
	
	char *copyFileName = strndup(pmanager->filename,(strlen(pmanager->filename))-3);
	strcat(copyFileName,"chk");
	if(copyFileName == NULL) {
		return -1;
	}
	
	int res = copy_file(processFilename(pmanager->filename,"stt"), processFilename(pmanager->filename,"chk"));
	
	return res;
}

/*-------------------------------------*/
char * processFilename(char * str, char * ext){
	char *copyFileName = strndup(str,(strlen(str))-3); // retira o \0
	strcat(copyFileName,ext);
	return copyFileName;
}


/* Retorna 1 caso existam dados no ficheiro de log e 0 caso contrário.
 */
int pmanager_has_data(struct pmanager_t *pmanager){
	return sizeFile(pmanager->file) > 0;
}
/**/
int sizeFile(FILE *file){
	struct stat f;
	int ret = fstat(fileno(file), &f); //MUDADO DE fstat(fileno(file, &f);
	if(ret == -1)
		return -1;

	return (int) f.st_size;
}

/*
 * RESTAURA A TABLE APARTIR DO FICHEIRO FORNECIDO DE ACORDO COM A EXTENSAO
 */
 int pmanager_restore_table(struct pmanager_t *pmanager,
                        struct table_t *table, char * ext){
	if(pmanager == NULL || table == NULL || ext == NULL){
		return -1;
	}
	
	char *msg_buf;

	FILE * load = fopen(processFilename(pmanager->filename, ext), "rb");
	

	int result = read_all2(fileno(load),&msg_buf);

	//printf("RESULT: %d", result);
	while(result > 0){

	struct message_t *msg;
	msg = buffer_to_message(msg_buf, result);
	if(msg == NULL){
		return -1;
	}
	switch(msg->opcode){
		case OC_DEL:
			if(table_del(table,msg->content.key)!=0){
				free(msg);
				fclose(pmanager->file);
				return -1;
			}
			break;
		case OC_PUT:
			if(table_put(table,msg->content.entry->key, msg->content.entry->value)!=0){
				free(msg);
				fclose(pmanager->file);
				return -1;
			}
			break;
		case OC_UPDATE:
			if(table_update(table,msg->content.entry->key, msg->content.entry->value)!=0){
				free(msg);
				fclose(pmanager->file);
				return -1;
			}
			break;
		default:
			free(msg);	
			fclose(pmanager->file);
			return -1;
			break;
	}
		free(msg);
		free(msg_buf);
		result = read_all2(fileno(load),&msg_buf);
	}
	if(result < 0){
		fclose(pmanager->file);
		return -1;
	}
	fclose(load); // fecha o modo de leitura

	return 0;
}
