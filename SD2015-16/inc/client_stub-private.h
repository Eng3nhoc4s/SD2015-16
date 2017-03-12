#ifndef _CLIENT_STUB_PRIVATE_H
#define _CLIENT_STUB_PRIVATE_H

#include "network_client.h"

#define OC_RT_GETTS 60
#define OC_NUM_OPS 70

struct rtable_t {
	struct server_t* server;
};

/* Função para obter o timestamp do valor associado a essa chave.
* Em caso de erro devolve -1. Em caso de chave não encontrada
* devolve 0.
*/
long long rtable_get_ts(struct rtable_t *rtable, char *key);

/*Funçao que devolve o numero de operações efectuadas numa tabela
*/
int rtable_num_ops(struct rtable_t *rtable);



#endif
