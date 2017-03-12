/* PROJECTO 1 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */
#ifndef _LISTPRIVATE_H
#define _LISTPRIVATE_H
#include "data.h"
#include "entry.h"
#include "list.h"

//Representa um Nó da lista contendo uma estrutura entry e um apontador para o nó seguinte
struct node_t {
	struct entry_t *par; 
	struct node_t *next;
};

// Esta estrutura define uma lista ligada
struct list_t {
	int size;
	struct node_t *first;

};

struct node_t *node_create(struct entry_t *entry);

void node_destroy(struct node_t *node);

/* Devolve um array de entry * com a cópia de todas as entries da 
 * tabela, e um último elemento a NULL.
 */
struct entry_t **list_get_entries(struct list_t *list);

/* Liberta a memoria reservada por list_get_entries.
 */
void list_free_entries(struct entry_t **entries);

#endif

