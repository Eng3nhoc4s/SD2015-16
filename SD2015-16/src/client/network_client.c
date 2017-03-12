/* PROJECTO 3 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include "network_client.h"
#include "network_client-private.h"
#include "communication.h"
#include "message.h"
#include <string.h>
#include <unistd.h>


/*Divide um string do formato <hostname>:<port>,
 * em duas strings <hostname> e <port> e coloca-as num array de Strings */
char **processString(char *address_port) {
	char **arrayStrings = (char**) malloc(sizeof(char *)* 2);
	char *token = strtok(address_port, ":");
	int i = 0;
	while(token != NULL) {
		arrayStrings[i] = strdup(token);
		if(arrayStrings[i] == NULL) {
			int j;
			for(j = 0; j < i; j++) {
				free(arrayStrings[j]);
			}
			free(arrayStrings);
			return NULL;
		}
		token = strtok(NULL, ":");
		i++;
	}
	return arrayStrings;
}

/*liberta a memoria alocada parara um array de Strings*/
void free_arrayStrings( char** processString) {
	if(processString != NULL) {
		free(processString[0]);
		free(processString[1]);
		free(processString);
	}
}

 /* Esta função deve:
 *  - estabelecer a ligação com o servidor;
 *  - address_port é uma string no formato <hostname>:<port>
 *    (exemplo: 10.10.10.10:10000)
 *  - retornar toda a informacão necessária (e.g., descritor da
 *    socket) na estrutura server_t
 */
struct server_t *network_connect(const char *address_port) {

	struct server_t *serverCon = (struct server_t*) malloc (sizeof(struct server_t));
	if(serverCon == NULL)
		return NULL;

	// Cria socket TCP
    if ((serverCon->sockFD = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        free(serverCon);
        return NULL; 
    }

    //Processa string com endereço ip e porta
   char* address = strdup(address_port);
    if(address == NULL) {
    	close(serverCon->sockFD);
    	free(serverCon);
    	return NULL;
    }
    char ** arrayStrings = processString(address);
    if(arrayStrings == NULL || arrayStrings[1] == NULL) {
    	printf("Parametros fornecidos incorrectos! ./client <ip>:<porta>\n");
    	close(serverCon->sockFD);
    	free(serverCon);
    	free(address);
    	return NULL;
    }

    // Preenche estrutura server para estabelecer conexão
    serverCon->addr.sin_family = AF_INET;
    serverCon->addr.sin_port = htons(atoi(arrayStrings[1]));
    if (inet_pton(AF_INET, arrayStrings[0], &serverCon->addr.sin_addr) < 1) {
        perror("Erro ao coverter String para IP");
        close(serverCon->sockFD);
    	free(serverCon);
    	free(address);
    	free_arrayStrings(arrayStrings);
    	return NULL;
    }

    // Estabelece conexão com o servidor definido em server
    if (connect(serverCon->sockFD,(struct sockaddr *)&serverCon->addr, sizeof(serverCon->addr)) < 0) {
        perror("Erro ao conectar-se ao servidor");
        close(serverCon->sockFD);
    	free(serverCon);
    	free(address);
    	free_arrayStrings(arrayStrings);
    	return NULL;
    }

    //liberta memoria da duplicação das const char e do arrayStrings 
    free(address);
    free_arrayStrings(arrayStrings);

    return serverCon;

}

/* Faz a reconcexão ao servidor */
int reconnect(struct server_t *server){

	sleep(RETRY_TIME);
	close(server->sockFD);

	server->sockFD = socket(AF_INET, SOCK_STREAM, 0);
	if (server->sockFD < 0) {
        perror("Erro ao criar socket TCP");
        return -1; 
    }

    connect(server->sockFD,(struct sockaddr *)&server->addr, sizeof(server->addr));

    return 0;

}

/* Esta função deve
 * - Obter o descritor da ligação (socket) da estrutura server_t;
 * - enviar a mensagem msg ao servidor;
 * - receber uma resposta do servidor;
 * - retornar a mensagem obtida como resposta ou NULL em caso
 *   de erro.
 */
struct message_t *network_send_receive(struct server_t *server, struct message_t *msg) {

	if(server == NULL || msg ==NULL) {
		printf("parametros network_send_receive a NULL");
		return NULL;
	}


	//transforma um message_t num buffer para enviar dados
	char * buf;
	int size = message_to_buffer(msg, &buf);
	if(size < 0) {
		printf("message_to_buffer size < 0\n");
		return NULL;
	}

	
	//envia os dados para o servidor
	int nbytes = write_all2(server->sockFD, buf, size);

	while(nbytes < 1) {
		int ret = reconnect(server);
	
		nbytes = write_all2(server->sockFD, buf, size);

	}

	if(nbytes != size){
		printf("FALHA: Operação Abortada!\n");
		free(buf);
		return NULL;
	}
	


	//recebe resposta do servidor
	free(buf);

	nbytes = read_all2(server->sockFD, &buf);

	if(nbytes <= 0) {
		return NULL;
	}


	//transforma os dados recebidos numa estrutura message_t
	struct message_t *receive = buffer_to_message(buf, nbytes);
	free(buf);

	return receive;

}

/* A função network_close() deve fechar a ligação estabelecida por
 * network_connect(). Se network_connect() alocou memória, a função
 * deve libertar essa memória.
 */
int network_close(struct server_t *server) {
	if(server == NULL)
		return -1;

	int cl = close(server->sockFD);
	free(server);

	return cl;

}


