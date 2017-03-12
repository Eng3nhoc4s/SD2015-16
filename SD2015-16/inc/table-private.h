/* PROJECTO 2 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#ifndef _TABLEPRIVATE_H
#define _TABLEPRIVATE_H

#include "list.h"

struct table_t  {
	struct list_t ** table; //apontador para array de list
	int capacity;  //tamanho do array de list
	int size;     //numero de elementos na tabela
	int nChanges; // num de aleracoes na tabela
};


/*funçao de hash: a partir da chave devolve um indice na tabela 
*/ 
int hash (char *key, int n);

/* Retorna o número de alterações realizadas na tabela.
*/
int table_get_num_change_ops(struct table_t *table);

/* Devolve um array de entry_t* com cópias de todas as entries
* da tabela, e um último elemento a NULL.
*/
struct entry_t **table_get_entries(struct table_t *table);

/* Liberta a memória alocada por table_get_entries().
*/
void table_free_entries(struct entry_t **entries);

/* Função para obter o timestamp do valor associado a uma chave.
* Em caso de erro devolve -1. Em caso de chave não encontrada devolve 0.
*/
long long table_get_ts(struct table_t *table, char *key);


#endif