/* PROJECTO 3 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include "message.h"
#include "message-private.h"
#include "table.h"
#include "network_client.h"
#include "data.h"
#include "entry.h"
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include "quorum_table.h"
#include "quorum_table-private.h"

#define MAX_LENGTH 2048

/*
 * A ação associada à receção de SIGPIPE passa a ser ignorar.
 */
int ignsigpipe(){
	struct sigaction s;

	s.sa_handler = SIG_IGN;
	return sigaction(SIGPIPE, &s, NULL);
}


void printMenu() {
	printf("\nLista de comandos:\n");
	printf("\tput <key> <data>\n"
			"\tget <key>\n"
			"\tdel <key>\n"
			"\tupdate <key> <data>\n"
			"\tsize\n"
			"\tquit\n"
			"\n"
			"Comando: ");
}

short verificaInput(char *comando, char *arg2, char *arg3) {

	if(comando == NULL)
		return -5;

	if (strcmp(comando, "put") == 0 ) 
		return arg2 != NULL && arg3 != NULL  && strcmp(arg2, "!") != 0 ? OC_PUT : -5;

	if (strcmp(comando, "get") == 0) 
		return arg2 != NULL && arg3 == NULL ? OC_GET : -5;

	if (strcmp(comando, "del") == 0) 
		return arg2 != NULL && arg3 ==NULL ? OC_DEL: -5;

	if (strcmp(comando, "update") == 0 )
		return arg2 != NULL && arg3 != NULL ? OC_UPDATE : -5;

	if (strcmp(comando, "size") == 0)
		return arg2 == NULL ?  OC_SIZE : -5;

	if (strcmp(comando, "quit") == 0)
		return arg2 == NULL ?  -1 : -5;

	return -5;
}

int main(int argc, char **argv) {


 	// Verifica argumentos
	if (argc < 2) {
    	printf("Uso: ./<nome_executavel>  <id_client>  <ip_server0>:<port0>  <ip_server1>:<port1> ....\n");
    	printf("Exemplo de uso: ./client  10 127.0.0.1:12340 127.0.0.1:12341 127.0.0.1:12342\n");
 		return -1;
 	}
 	int argSize = 0;
 	int index = 2;
 	char ** addresses_ports = (char **)malloc(sizeof(char *) * (argc-2));
 	if(addresses_ports == NULL)
 		return -1;

 	while( index < argc) {
 		addresses_ports[argSize] = strdup(argv[index]);
 		
 		if(addresses_ports[argSize] == NULL) {
 			int j;
 			for(j = 0; j < argSize; j++) 
 				free(addresses_ports[j]);
 			free(addresses_ports);
 			return -1;
 		}

 		argSize ++;
 		index++;		
 	}
 	
 	short opcode;
 	//conecta-se com o servidor
 	struct qtable_t *qtable = qtable_bind(addresses_ports, argSize);
 	if(qtable == NULL) {
 		free(addresses_ports);
 		return -1;
 	}
 	qtable->client_id = atoi(argv[1]);

 	if (ignsigpipe() != 0){ 
			printf("ignsigpipe falhou\n");
			free(addresses_ports);
			qtable_disconnect(qtable);
			return -1;
		}

	char *key, *data;
	int ret;
	
	do {
	//Imprimir o menu
	printMenu();
	
		//Ler input do utilizador 
		char usrInput[MAX_LENGTH]; 	
		fgets(usrInput, MAX_LENGTH, stdin);
		
		//Separa input em 3 strings possiveis
		char * arg[3];
		arg[0] = strtok(usrInput, " \n");
		
		arg[1] = strtok(NULL, " \n");
		
		arg[2] = strtok(NULL, "\n");
		

		//Verifica argumentos do input do utilizador e devolve um opcode da operaçao
		opcode = verificaInput(arg[0], arg[1], arg[2]);

		
		//Trata do comando selecionado pelo utilizador 
		switch (opcode) {
		
		
			case OC_PUT:
				printf("** OPERACAO -- PUT -- SELECIONADA **\n");
				
				key = strdup(arg[1]);
				data = strdup(arg[2]);
				struct data_t *dataPUT = data_create2(strlen(data) + 1, data);
				
				if(dataPUT == NULL){
					printf("Falha na iserção de uma nova entrada na remote table!\n");
					free(key);
					free(data);
					break;
				}	
							
				ret = qtable_put(qtable, key, dataPUT);
				
				printf("RESULT PUT = %d  ->0 OK, -1 NOT OK\n ", ret );
				
				data_destroy(dataPUT);
				free(key);
				free(data);
				break;

			//GET
			case OC_GET:

				printf("** OPERACAO -- GET -- SELECIONADA **\n");
				key = strdup(arg[1]);
				
				//GET 1 KEY
				if(strcmp(arg[1], "!") != 0){
					
					struct data_t *d = qtable_get(qtable, key); //ARMAZENA A DATA OBTIDA

					
					if(d == NULL){
						printf("Erro ao obter a chave na remote table!\n");
					
					}else{	
						printf("RESULT GET = %s \n ", (char*) d->data);			
						data_destroy(d); 						//PARA NAO TER MEMORY LEAKS
					}
				
				//GET ALL KEYS
				}else{

					char ** allKeys = qtable_get_keys(qtable);

					if(allKeys == NULL){
						printf("Erro ao obter a lista de chaves!\n");
					
					}else{
					
						printf("RESULT GET_KEYS =\n ");
						int i = 0;
						while(allKeys[i] != NULL){
							printf("KEY: %s\n",allKeys[i] );
							i++;
						}
					
						qtable_free_keys(allKeys);				
					}
					
				
				}
				
				free(key);
				break;

			case OC_DEL:

				printf("** OPERACAO -- DEL -- SELECIONADA **\n");
				
				key = strdup(arg[1]);
				
				ret = qtable_del(qtable, key);
				
				printf("RESULT DEL = %d  ->0 OK, -1 NOT OK\n ", ret );
					
				free(key);
				
				break;

			case OC_UPDATE:

				printf("** OPERACAO -- UPDATE -- SELECIONADA **\n");
				
				key = strdup(arg[1]);
				data = strdup(arg[2]);
				
				struct data_t *dataUP = data_create2(strlen(data) + 1, data);
				
				if(dataUP == NULL){
					printf("Erro ao actualizar entrada na remote table!\n");
					free(key);
					free(data);
					break;
				}
				
				ret = qtable_update(qtable, key, dataUP);
				
				printf("RESULT UPDATE = %d  ->0 OK, -1 NOT OK\n ", ret );
				
				data_destroy(dataUP);
				free(key);
				free(data);
				break;

			case OC_SIZE:

				printf("** OPERACAO -- SIZE -- SELECIONADA **\n");
				
				ret = qtable_size(qtable);

				printf("RESULT SIZE = %d\n ", ret );
				
				break;

			case -1:
				break;

			default:
				printf("\nErro! Comando Inválido!\n\n");
		}

	} while (opcode != -1);
	
	free(addresses_ports);
	qtable_disconnect(qtable);
 	printf("FIM\n");

 	return 0;

 }
