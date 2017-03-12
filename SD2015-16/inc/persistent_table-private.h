#ifndef _PERSISTENCE_TABLE_PRIVATE_H
#define _PERSISTENCE_TABLE_PRIVATE_H

struct ptable_t {
	struct table_t *table;
	struct pmanager_t *pmanager;
	int open;
};

/* Função para obter o timestamp do valor associado a uma chave.
* Em caso de erro devolve -1. Em caso de chave não encontrada
* devolve 0.
*/
long long ptable_get_ts(struct ptable_t *ptable, char *key);

/*Funçao que devolve o numero de operações efectuadas numa tabela
*/
int ptable_num_ops(struct ptable_t *ptable);

#endif
