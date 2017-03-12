/* PROJECTO 3 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#ifndef _NETWORK_CLI_PRIVATE_H
#define _NETWORK_CLI_PRIVATE_H

#include "inet.h"

#define RETRY_TIME 2

struct server_t  {
	int sockFD; 	//File Descriptor do socket
	struct sockaddr_in addr;    //Estrutura que guarda o ip adress e o porto
};

/*Divide um string do formato <hostname>:<port>,
 * em duas strings <hostname> e <port> e coloca-as num array de Strings */
char **processString(char *address_port);

/*liberta a memoria alocada parara um array de Strings*/
void free_arrayStrings( char** processString);

/* Faz a reconcexão ao servidor */
int reconnect(struct server_t *server);
#endif
