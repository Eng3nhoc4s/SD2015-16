/* PROJECTO 4 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */
#define _GNU_SOURCE //Para retirar o warning do get_current_dir_name()
#include "message.h"
#include "message-private.h"
#include "table.h"
#include "communication.h"
#include "inet.h"
#include "table_skel-private.h"
#include "table_skel.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>


#define MAXLIG 1024 //numero maximo de ligações
#define TIMEOUT 500 //tempo de espera em ms

/*
 * A ação associada à receção de SIGPIPE passa a ser ignorar.
 */
int ignsigpipe(){
	struct sigaction s;

	s.sa_handler = SIG_IGN;
	return sigaction(SIGPIPE, &s, NULL);
}

/*
 *retorna um indice livre no array de estruturas pollfd
*/
int procuraIndiceLivre(struct pollfd *poll) {
	int i = 0;
	while(i < MAXLIG) {
		if(poll[i].fd ==-1)
			return i;
		i++;
	}

	return 1;
}

/*Cria um socket para comunicação com o exterior*/
/*Devolve o fd do scket criado ou -1 em caso de erro*/
int create_connection( int porto ) {

	//cria um socket
	int socketFD = socket(AF_INET,SOCK_STREAM, 0);
	if(socketFD == -1) {
		perror("Erro criacao de socket");
		return -1;
	}

	int sim = 1;
	if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, (int*)&sim,sizeof(sim)) < 0 ) {
		perror("SO_REUSEADDR setsockopt error");
		return -1;
	}

	//Preenche estrutura com adresses para o bind
	struct sockaddr_in server;
	server.sin_family = AF_INET;
    server.sin_port = htons(porto);
    server.sin_addr.s_addr = htonl(INADDR_ANY);


    // Faz bind
    if (bind(socketFD, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("Erro ao fazer bind");
        close(socketFD);
        return -1;
    }


    // Faz listen
    if (listen(socketFD, 0) < 0){
        perror("Erro ao executar listen");
        close(socketFD);
        return -1;
    }

    return socketFD;
}

int main(int argc, char **argv){


	// Verifica se foi passado algum argumento
	if (argc != 4) {
    	printf("Uso: ./<nome_executavel> <porto_servidor> <num_listas> <path_ficheiro_log>\n");
    	
    	char * currDir = get_current_dir_name();
    	if(currDir == NULL) {
    		printf("Exemplo de uso: ./server 12345 12 caminho/table.log\n");
    		return -1;
    	}
    	printf("Exemplo de uso: ./server 12345 12 %s/table.log\n", currDir);
    	free(currDir);
 		return -1;
 	}

 	if (ignsigpipe() != 0){ 
			perror("ignsigpipe falhou");
			return -1;
		}

 	int tableret = table_skel_init( atoi(argv[2]), argv[3]);
 	if(tableret == -1) {
 		printf("Erro a iniciar table\n");
 		return -1;
 	}

 	//cria socket de escuta, faz bind e listen
 	int socket_id = create_connection( atoi(argv[1]) );
 	if(socket_id < 0) {
 		perror("Erro a criar coneccao");
 		return -1;
 	}

 	
 	printf("Servidor à espera de dados\n");
 	
 	
 	struct sockaddr_in client;
 	socklen_t size_client;
 	struct pollfd connections[MAXLIG];
 	int numSockets = 0;
 	int i, ret, bufferSize, indice, tamanho;
 	char *bufferLido; 
 	char * msg_buf;
 	struct message_t *pedido;
 	struct message_t *resposta;


 	//"inicializaçao" das estruturas pollfd
 	for (i = 0; i < MAXLIG; i++)
    connections[i].fd = -1;

    //colocacao da socket de escuta na primeira posiçao do array
    connections[0].fd = socket_id;
    connections[0].events = POLLIN;
  

    numSockets++; 

    //Fica a aguardar eventos
    while ( (ret = poll(connections, numSockets, TIMEOUT)) >= 0) {

    	//PARA APAGAR
    	//printf("numSockets = %d\n", numSockets);

		if(ret > 0) {  // existe algum socket a pedir "atenção".

			//verifica se é o de escuta
			if((connections[0].revents & POLLIN) && (numSockets < MAXLIG)) {
				printf("SERVIDOR DE ESCUTA AO TRABALHO\n");

				indice = procuraIndiceLivre(connections);
		
				if((connections[indice].fd = accept(connections[0].fd, (struct sockaddr *) &client, &size_client)) > 0) {
					connections[indice].events = POLLIN;
					numSockets++;
				}
				else {
					perror("Erro no Accept");
					return -1;
				}
				ret--;
			}
			
			//verifica se é nos outros sockets
			for(i = 1; i < MAXLIG && ret > 0; i++) {

				if ((connections[i].fd > -1) && (connections[i].revents & POLLIN)) { // Dados para ler ?

					printf("DADOS PARA LER EM %i\n", i);
					//lê o que o cliente envia
					bufferSize = read_all2(connections[i].fd, &bufferLido);
					if(bufferSize > 0) {

						//Caso tenha lido, desserializa a mensagem
						pedido = buffer_to_message(bufferLido, bufferSize);
						if(pedido == NULL) {
							free(bufferLido);
							continue;
						}
						//Envia para skeleton para tratar do pedido
						resposta = invoke(pedido);
						if(resposta == NULL) {
							free(bufferLido);
							free_message(pedido);
							continue;
						}

						//Serializa a resposta
						tamanho = message_to_buffer(resposta, &msg_buf);
						if(tamanho < 0) {
							free_message(pedido);
							free_message(resposta);
							free(bufferLido);
							continue;
						}

						// Envia resposta ao cliente pelo socket referente a conexão
						tamanho= write_all2(connections[i].fd, msg_buf, tamanho);
						if(tamanho < 1) {
							free_message(pedido);
							free_message(resposta);
							free(bufferLido);
							free(msg_buf);
							continue;
						}

						printf("MENSAGEM ENTREGUE\n");
						//Limpa a memoria
						free_message(pedido);
						free_message(resposta);
						free(bufferLido);
						free(msg_buf);
					}
					else {
						close(connections[i].fd);
						connections[i].fd = -1;
						numSockets--;
					}

					
				}
				//Verifica outros eventos( Erros)
				if( (connections[i].fd > -1) && ((connections[i].revents == POLLERR) || (connections[i].revents == POLLHUP))) {
					close(connections[i].fd);
					connections[i].fd = -1;
					numSockets--;

				}

			}

		}

	}//fim do while

	
	table_skel_destroy();


	//Fecha Todas as ligacoes
	for(i = 1; i < MAXLIG; i++) {

		if(connections[i].fd > -1) {
			close(connections[i].fd);
		}
	}
	printf("FIM DE EXECUÇÃO\n");

	return 0;
}
