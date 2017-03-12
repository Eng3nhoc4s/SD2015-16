/* Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */
#ifndef _QUORUM_TABLE_PRIVATE_H
#define _QUORUM_TABLE_PRIVATE_H

#include "client_stub.h"
#include "client_stub-private.h"


struct qtable_t {
	struct rtable_t* rtables;
	int client_id;
	int n_servers;
};

/*Incrementa o valor de um timestamp dado um cliente id*/
long long increment_ts( long long ts, int id);

int ceil2(int n);

#endif