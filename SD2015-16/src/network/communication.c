/* PROJECTO 3 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

 #include <unistd.h>
 #include <stdlib.h>
 #include "inet.h"

/*Lê buffer de comunicação e guarda a informaçao num apontador
 * Retorna o nº de bytes lido, 0 caso a comunicacão falhe ou -1 em caso de erro
 */
int read_all( int sockfd, void * pointer, int size ) {

  	int nbytes;
  	while(size > 0) {
		nbytes = read(sockfd, pointer, size);
		if(nbytes < 0) {
			perror("Erro a ler do buffer");
			return -1;
		}
		//verifica se cliente fechou ligação
		if(nbytes == 0) {
			printf("Ligacao foi terminada\n");
			return 0;
		}

		pointer += nbytes;
		size -= nbytes; 
	}
	return nbytes;
}


/*Escreve num buffer de comunicação a informaçao da variavel pointer
 *Retorna o numero de bytes escrito, -1 em caso de erro ou 0 caso ligaçao tenha terminado
 */
int write_all( int sockfd, void * pointer, int size ) {

	int nbytes;
	while(size > 0){
		nbytes = write(sockfd,pointer, size);
		if(nbytes < 0) {
			perror("Erro a escrever tamanho dos dados para buffer");
			return -1;
		}
		
		size-= nbytes;
		pointer += nbytes;
	}

	return nbytes;
}


/*envia para o cliente a resposta do servidor*/
/*Protocolo: envia primeiro o tamanho da mensagem a enviar(4 bytes) e depois envia a mensagem*/ 
/*Retorna o tamanho do buffer com a mensagem  enviado -1 em caso de erro */ 
int write_all2( int sockID, char *msg, int bufferSize) {

	int nBytes;
	int convert = htonl(bufferSize);
	int *p = &convert;
	nBytes = write_all(sockID,p, 4);

	if(nBytes < 0) {
		return -1;
	}

	nBytes = write_all(sockID, msg, bufferSize);

	return nBytes;
}

/*Lê todo buffer de comunicação e guarda a informaçao em msg_buffer
 *Protocolo: lê 4 bytes com o tamanho da mensagem e de seguida lê mensagem com esse tamaho.
 *Retorna o tamanho do buffer com a mensagem,-1 em caso de erro, 
 * ou 0 caso cliente tenha fechado ligação*/ 
int read_all2( int sockfd, char ** msg_buf ) {

	int size;
	int *sizeP = &size;
	int nBytes = read_all(sockfd, sizeP, 4);
	if(nBytes == 0) {
		printf("Nao foi possivel ler\n");
		return 0;
	}
	if(nBytes == -1) {
		return -1;
	}
	size = ntohl(*sizeP);
	

	*msg_buf = (char *) malloc(size);
	if(*msg_buf == NULL) {
		perror("Erro a criar buffer com pedido do cliente");
		return -1;
	}

	nBytes = read_all(sockfd, *msg_buf, size);

	if(nBytes == 0) {
		free(*msg_buf);
		return 0;
	}
	if(nBytes == -1) {
		free(*msg_buf);
		return -1;
	}

	return nBytes;
}


