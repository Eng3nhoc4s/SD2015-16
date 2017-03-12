/* Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */
#include "data.h"
#include "table.h"
#include "client_stub-private.h"

/* Remote table. A definir pelo grupo em client_stub-private.h 
 */
struct rtable_t; 

/* Função para estabelecer uma associação entre o cliente e uma tabela
 * remota num servidor.
 * address_port é uma string no formato <hostname>:<port>.
 * retorna NULL em caso de erro .
 */
struct rtable_t *rtable_bind(const char *address_port){
	
	if(address_port == NULL){
		printf("Parametros inválidos rtable_bind\n");
		return NULL;
	}
	
	struct rtable_t *rtable = (struct rtable_t *) malloc (sizeof(struct rtable_t));
	
	if(rtable == NULL){
		printf("Erro ao alocar memoria rtable_t\n");
		return NULL;
	}
	
	rtable->server = network_connect(address_port);
	
	if(rtable->server == NULL){
		free(rtable);
		printf("Falha na conexão ao servidor!\n");
		return NULL;
	}
	
	return rtable;
}

/* Termina a associação entre o cliente e a tabela remota, e liberta
 * toda a memória local. 
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtable_unbind(struct rtable_t *rtable){

	if(rtable == NULL){
		printf("Falha no encerramento da conexão entre o cliente e o servidor!\n");
		return -1;
	}
	
	int res = network_close(rtable->server);
	
	if(res == -1){
		free(rtable);
		printf("Falha no encerramento da conexão entre o cliente e o servidor\n");
		return -1;
	}
		
	free(rtable);
	return 0;
}

/* Função para adicionar um par chave valor na tabela remota.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int rtable_put(struct rtable_t *rtable, char *key, struct data_t *value){

	if(rtable == NULL || key == NULL || value == NULL){
		printf("Parametros inválidos rtable_put\n");	
		return -1;
	}
	
	//DUP KEY
	char * duppedKey = strdup(key);
	
	if(duppedKey == NULL){
		printf("Erro alocação de memoria para a duplicação de chave\n");
		return -1;
	}
	
	//DUP DATA
	struct data_t *duppedData = data_dup(value);
	
	if(duppedData == NULL){
		printf("Erro alocação de memoria para a duplicação de data\n");
		free(duppedKey);
		return -1;
	}
	
	//CRIACAO DA MENSAGEM
	struct message_t * msgPut = (struct message_t*) malloc(sizeof(struct message_t));
	
		if(msgPut == NULL){
			printf("Erro alocação de memoria para a mensagem\n");
			free(duppedKey);
			data_destroy(duppedData);
			return -1;
		}
		msgPut->opcode = OC_PUT;
		msgPut->c_type = CT_ENTRY;
		msgPut->content.entry = entry_create(duppedKey, duppedData);
	
	//ENVIO DA MENSAGEM
	struct message_t * putResposta = network_send_receive(rtable->server,msgPut);
		if(putResposta == NULL) {
			free_message(msgPut);
			printf("Não foi possivel contactar servidor\n");
			return -1;
		}
	

		int result = putResposta->content.result;
	
	//FREES
	free(duppedKey);
	data_destroy(duppedData);
	free_message(msgPut);
 	free_message(putResposta);
 	
 	return result;
}

/* Função para substituir na tabela remota, o valor associado à chave key.
 * Devolve 0 (OK) ou -1 em caso de erros.
 */
int rable_update(struct rtable_t *rtable, char *key, struct data_t *value){	//TEM UM ERRO O PROF "RABLE_UPDATE" EM VEZ E "RTABLE_UPDATE"

	if(rtable == NULL || key == NULL || value == NULL){
		printf("Parametros inválidos rtable_upate\n");	
		return -1;
	}
	
	//DUP KEY
	char * duppedKey = strdup(key);
	
	if(duppedKey == NULL){
		printf("Erro alocação de memoria para a duplicação de chave\n");
		return -1;
	}
	
	//DUP DATA
	struct data_t *duppedData = data_dup(value);
	
	if(duppedData == NULL){
		printf("Erro alocação de memoria para a duplicação de data\n");
		free(duppedKey);
		return -1;
	}
	
	//CRIACAO DA MENSAGEM
	struct message_t * msgUp = (struct message_t*) malloc(sizeof(struct message_t));
	
		if(msgUp == NULL){
			printf("Erro alocação de memoria para a mensagem\n");
			free(duppedKey);
			free(duppedData);
			return -1;
		}
		
 		msgUp->opcode = OC_UPDATE;
 		msgUp->c_type = CT_ENTRY;
 		msgUp->content.entry = entry_create(duppedKey, duppedData);

	//ENVIO DA MENSAGEM
	struct message_t *uprps = network_send_receive(rtable->server,msgUp);
		
		if( uprps== NULL) {
			free_message(msgUp);
			printf("Não foi possivel contactar servidor\n");
			return -1;
		}

			
	//FREES
	free(duppedKey);
	data_destroy(duppedData);
	free_message(msgUp);
	free_message(uprps);
	
	return 0;
}

/* Função para obter da tabela remota o valor associado à chave key.
 * Devolve NULL em caso de erro.
 */
struct data_t *rtable_get(struct rtable_t *table, char *key){	//ATENCAO O PROF NESTA FC CHAMA "TABLE" EM VEZ DE
																//"RTABLE" À STRUCT RTABLE_T
	if(table == NULL || key == NULL){
		printf("Parametros inválidos rtable_get\n");	
		return NULL;
	}
	
	//DUP KEY
	char * duppedKey = strdup(key);
	
	if(duppedKey == NULL){
		printf("Erro alocação de memoria para a duplicação de chave\n");
		return NULL;
	}
	
	//CRIACAO DA MENSAGEM
	struct message_t * msgGet = (struct message_t*) malloc(sizeof(struct message_t));
	
		if(msgGet == NULL){
			printf("Erro alocação de memoria para a mensagem\n");
			free(duppedKey);
			return NULL;
		}
	 	msgGet->opcode = OC_GET;
	 	msgGet->c_type = CT_KEY;
	 	msgGet->content.key = duppedKey;

	//ENVIO DA MENSAGEM
 	struct message_t * getrsp = network_send_receive(table->server,msgGet);
	 	
	 	if(getrsp == NULL) {
			free_message(msgGet);
			printf("Não foi possivel contactar servidor\n");
			return NULL;
		}
		
 	
 	struct data_t *toReturn = data_dup(getrsp->content.data);
 	
 	//FREES
 	//free(duppedKey); JA E LIBERTADO PELO FREE_MESSAGE(msgGET);
 	free_message(msgGet);
 	free_message(getrsp);
 	
 	//SE toReturn der raia, o seu resultado é NULL
 	return toReturn;
}

/* Função para remover um par chave valor da tabela remota, especificado 
 * pela chave key.
 * Devolve: 0 (OK) ou -1 em caso de erros.
 */
int rtable_del(struct rtable_t *table, char *key){				
															
	if(table == NULL || key == NULL){			
		printf("Parametros inválidos rtable_del\n");	
		return -1;
	}
	
	//DUP KEY
	char * duppedKey = strdup(key);
	
	if(duppedKey == NULL){
		printf("Erro alocação de memoria para a duplicação de chave\n");
		return -1;
	}
	
	//CRIACAO DA MENSAGEM
	struct message_t * msgDel = (struct message_t*) malloc(sizeof(struct message_t));
	
		if(msgDel == NULL){
			printf("Erro alocação de memoria para a mensagem\n");
			free(duppedKey);
			return -1;
		}
	 	msgDel->opcode = OC_DEL;
	 	msgDel->c_type = CT_KEY;
	 	msgDel->content.key = duppedKey;
	 	
	 //ENVIO DA MENSAGEM
	 struct message_t * delrsp = network_send_receive(table->server,msgDel);
	 	
	 	if(delrsp == NULL) {
			free_message(msgDel);
			printf("Não foi possivel contactar servidor\n");
			return -1;
		}
		
		
 	
 	//FREES
 	//free(duppedKey);
 	free_message(msgDel);
 	free_message(delrsp);
 	
 	return 0;
}
/* Função para obter o timestamp do valor associado a essa chave.
* Em caso de erro devolve -1. Em caso de chave não encontrada
* devolve 0.
*/
long long rtable_get_ts(struct rtable_t *rtable, char *key){

	printf("<<<rtable_get_ts>>>> Key = %s\n", key);

	if(rtable == NULL){
		printf("Parametros inválidos rtable_get_ts\n");	
		return -1;
	}
	
	//CRIACAO DA MENSAGEM
	struct message_t * msgTs = (struct message_t*) malloc(sizeof(struct message_t));
	
		if(msgTs == NULL){
			printf("Erro alocação de memoria para a mensagem\n");
			return -1;
		}

		char * dupKey = strdup(key);
		if(dupKey == NULL) {
			free(msgTs);
			return -1;
		}
	
	 	msgTs->opcode = OC_RT_GETTS;
	 	msgTs->c_type = CT_KEY;
	 	msgTs->content.key = dupKey;

	//ENVIO DA MENSAGEM
 	struct message_t * sizersp = network_send_receive(rtable->server,msgTs);
 	
	 	if(sizersp == NULL) {
			free_message(msgTs);
			printf("Não foi possivel contactar servidor\n");
			return -1;
		}
		
	
 	long long timestamp = sizersp->content.timestamp;
 		
 	//FREES
 	free_message(msgTs);
 	free_message(sizersp);


	return timestamp;
}

/* Devolve número de elementos na tabela remota.
 */
int rtable_size(struct rtable_t *rtable){

	if(rtable == NULL){
		printf("Parametros inválidos rtable_size\n");	
		return -1;
	}
	
	//CRIACAO DA MENSAGEM
	struct message_t * msgSize = (struct message_t*) malloc(sizeof(struct message_t));
	
		if(msgSize == NULL){
			printf("Erro alocação de memoria para a mensagem\n");
			return -1;
		}
	
	 	msgSize->opcode = OC_SIZE;
	 	msgSize->c_type = CT_RESULT;

	//ENVIO DA MENSAGEM
 	struct message_t * sizersp = network_send_receive(rtable->server,msgSize);
 	
	 	if(sizersp == NULL) {
			free_message(msgSize);
			printf("Não foi possivel contactar servidor\n");
			return -1;
		}

 	
 	int size = sizersp->content.result;
 		
 	//FREES
 	free_message(msgSize);
 	free_message(sizersp);

	if(size < 0)
 		return -1;

	return size;
}

/*Funçao que devolve o numero de operações efectuadas numa tabela
*/
int rtable_num_ops(struct rtable_t *rtable) {
	if(rtable == NULL){
		printf("Parametros inválidos rtable_size\n");	
		return -1;
	}
	
	//CRIACAO DA MENSAGEM
	struct message_t * msgOPS = (struct message_t*) malloc(sizeof(struct message_t));
	
		if(msgOPS == NULL){
			printf("Erro alocação de memoria para a mensagem\n");
			return -1;
		}
	
	 	msgOPS->opcode = OC_NUM_OPS;
	 	msgOPS->c_type = CT_RESULT;

	//ENVIO DA MENSAGEM
 	struct message_t * sizersp = network_send_receive(rtable->server,msgOPS);
 	
	 	if(sizersp == NULL) {
			free_message(msgOPS);
			printf("Não foi possivel contactar servidor\n");
			return -1;
		}
		
 	
 	int size = sizersp->content.result;
 		
 	//FREES
 	free_message(msgOPS);
 	free_message(sizersp);

	if(size < 0)
 		return -1;

	return size;

}

/* Devolve um array de char * com a cópia de todas as keys da
 * tabela remota, e um último elemento a NULL.
 */
char **rtable_get_keys(struct rtable_t *rtable){


	//OBTENCAO A MSG COM AS KEYS
	//CRIACAO DA MENSAGEM
	struct message_t * msgKeys = (struct message_t*) malloc(sizeof(struct message_t));
	
		if(msgKeys == NULL){
			printf("Erro alocação de memoria para a mensagem\n");
			return NULL;
		}
	
	 	msgKeys->opcode = OC_GET;
	 	msgKeys->c_type = CT_KEY;
	 	msgKeys->content.key = strdup("!");
	 	
	//ENVIO DA MENSAGEM
 	struct message_t * keysrsp = network_send_receive(rtable->server,msgKeys);
 	
	 	if(keysrsp == NULL) {
			free_message(msgKeys);
			printf("Não foi possivel contactar servidor\n");
			return NULL;
		}
	
	int size = rtable_size(rtable);
	
	char ** allKeys = (char **) malloc ((size + 1) * sizeof(char*));
	
	if(allKeys == NULL) {
		free_message(msgKeys);
		free_message(keysrsp);
		return NULL;
	}
	
	//DUPLICAR AS CHAVES PARA DEVOLVER
	int i;
	for(i = 0; i < size; i++){
		
		allKeys[i] = strdup(keysrsp->content.keys[i]);
		//printf("KEY: %s\n", allKeys[i]);
		//LIBERTAR TUDO EM CASO DE ERRO
		int j;
		if(allKeys[i] == NULL){
			for(j=0; j < i; j++){
				free(allKeys[j]);
			}
		}
	}
	
	allKeys[size] = NULL;
	
	//FREE
	free_message(msgKeys);
	free_message(keysrsp);

	return allKeys;

}

/* Liberta a memória alocada por table_get_keys().
 */
void rtable_free_keys(char **keys){

	table_free_keys(keys);
	
}
