/* PROJECTO 1 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include <stdlib.h>
#include <string.h>

#include "entry.h"
#include "list.h"
#include "list-private.h"

/* Cria um nó para a lista (contém um apontador para o elemento antes e após deste, 
 * mais um apontador para a data que este deve conter*/
struct node_t *node_create(struct entry_t *entry) {

	//Verificação dos parâmetros fornecidos
	if(entry == NULL)
		return NULL;

	//Alocação de memoria para a estrutura node
	struct node_t* newNode = (struct node_t*) malloc (sizeof(struct node_t));
	if(newNode == NULL)
		return NULL;

	//Duplicação da entry e atribuição da cópia ao node
	newNode->par = entry_dup(entry);

	//Verificação da integridade da duplicação da entry
	if(newNode->par == NULL){
		free(newNode);
		return NULL;
	}

	//Inicialização do valor do próximo node
	newNode->next = NULL;

	return newNode;
}

/* Destrói um nó da lista e liberta toda a sua memória
 */
void node_destroy(struct node_t *node) {

	if(node != NULL) {
		entry_destroy(node->par);
		free(node);
	}
}

/* Cria uma nova lista. Em caso de erro, retorna NULL.
 */
struct list_t *list_create() {

	//Alocação de memoria para a estrutura node
	struct list_t *newList = (struct list_t*) malloc (sizeof(struct list_t));
	
	//Verificação da integridade da alocação da estrutura list_t
	if(newList == NULL) 
		return NULL;

	//Inicialização dos atributos da lista
	newList->size = 0;
	newList->first = NULL;

	return newList;

}

/* Elimina uma lista, libertando *toda* a memoria utilizada pela
 * lista.
 */
void list_destroy(struct list_t *list) {

	//Verificação dos parâmetros fornecidos
	if(list != NULL) {

		//liberta toda a memoria usada pela lista(incluindo todos os nodes)
		if(list->size == 0) 
			free(list);
		else {
			struct node_t *current = list->first->next;
			while(current != NULL) {
				node_destroy(list->first);
				list->first = current;
				current = list->first->next;
			}
			node_destroy(list->first);
			free(list);
		}
	}	
}

/* Adiciona uma entry na lista. Como a lista deve ser ordenada, 
 * a nova entry deve ser colocada no local correto.
 * Retorna 0 (OK) ou -1 (erro)
 * Caso a entrada a chave a ser adicionada seja inferior ou igual é adicionada antes
 */
int list_add(struct list_t *list, struct entry_t *entry) {

	//Verificação dos parâmetros fornecidos
	if(list == NULL)
		return -1;

	//Alocação de memoria para a o nó a adicionar
	struct node_t *nodeToAdd = node_create(entry);

	//Verificação da integridade do nó a adicionar	
	if(nodeToAdd == NULL)
		return -1;

	//Caso a lista ainda não tenha nenhum elemento, adiciona o primeiro node
	if(list->first == NULL) {
		list->first = nodeToAdd;
		list->size++;
	
		return 0;
	}

	//Comparação da chave do nó a adicionar com a cabeça da lista
	int ret = strcmp(list->first->par->key, nodeToAdd->par->key);

	//Se a chave já existir, destroi-se o nó e devolve-se um erro
	if(ret == 0) {
	 	if( list->first->par->value->datasize == 0) {
			entry_destroy(list->first->par);
			list->first->par = entry_dup(entry);
			node_destroy(nodeToAdd);
			return 0;
		}
		else {
			node_destroy(nodeToAdd);
			return -1;
		}
	}

	//Se a chave do primeiro elemento da lista for > chave do nó a adicionar
	//o nodeToAdd substitui o first
	if(ret > 0)  {
		nodeToAdd->next = list->first;
		list->first = nodeToAdd;
		list->size++;
	
		return 0;
	}
	//Se a chave do nodeToAdd > first
	else {
		struct node_t *current = list->first;
		while(current->next != NULL) {
			ret = strcmp(current->next->par->key, nodeToAdd->par->key);
			if(ret == 0) {
				if(current->next->par->value->datasize == 0) {
					entry_destroy(current->next->par);
					current->next->par = entry_dup(entry);
					node_destroy(nodeToAdd);
					return 0;
				}
				else {
					node_destroy(nodeToAdd);
					return -1;
				}
			}

			// A chave do current->next é maior que a do nodeToAdd (intercala-se o novo node)
			if(ret > 0) { 
				nodeToAdd->next = current->next;
				current->next = nodeToAdd;
				list->size += 1;
				return 0;
			}
			else
				current = current->next;
		}
		
		//Adiciona-se o node no fim da lista
		current->next = nodeToAdd;
		list->size += 1;
	} 

	return 0;
}

/* Elimina da lista um elemento com a chave key. 
 * Retorna 0 (OK) ou -1 (erro)
 */
int list_remove(struct list_t *list, char* key) {
	
	//Verificação dos parâmetros fornecidos
	if(list==NULL || key == NULL)
		return-1;

	//Verifica se a lista tem nodes
	if(list->first == NULL)
		return -1;

	//Comparação com a Key do primeiro node
	int ret = strcmp(list->first->par->key, key);
	struct node_t *aux;
	//Se a chave foi achada ao inicio da lista
	if(ret == 0) {
		aux = list->first->next;
		node_destroy(list->first);
		list->first = aux;
		list->size -= 1;
		return 0;
	}
	//Caso contrário continua a percorrer e a testar elementos da lista
	else {
		struct node_t *current = list->first;
		while(current->next != NULL) {
			ret = strcmp(current->next->par->key, key);
			if(ret == 0) {
				aux = current->next->next;
				node_destroy(current->next);
				current->next = aux;
				list->size -= 1;
				return 0;
			}
			else 
				current = current->next;
		}
		
		//Chegou ao fim e não encontrou o nó corresponde
		return -1;

	}
}

/* Obtem um elemento da lista que corresponda à chave key. 
 * Retorna a referência do elemento na lista (ou seja, uma alteração
 * implica alterar o elemento na lista). 
 */
struct entry_t *list_get(struct list_t *list, char *key) {

	//Verificação dos parâmetros fornecidos
	if(list == NULL || key == NULL)
		return NULL;

	struct node_t *current = list->first;
	
	//Percorre os nodes da lista até encontar um com a Key pretendida
	while(current != NULL) {
		int ret = strcmp(current->par->key, key);
		if(ret == 0) {
			return current->par;
		}
		else
		current = current->next;
	}

	//A key não foi encontrada na lista
	return NULL;

}

/* Retorna o tamanho (numero de elementos) da lista 
 * Retorna -1 em caso de erro.  */
int list_size(struct list_t *list) {
	return list == NULL ? -1 : list->size;
}

/* Devolve um array de char * com a cópia de todas as keys da 
 * tabela, e um último elemento a NULL.
 */
char **list_get_keys(struct list_t *list) {
	//Verificação dos parâmetros fornecidos
	if(list == NULL )
		return NULL;

	//Alocação de memória para a lista de apontadores
	char **listPointer = (char **) malloc (sizeof (char*) * (list->size + 1));

	//Verificação da integridade da alocação da lista de apontadores
	if(listPointer == NULL)
		return NULL;

	struct node_t *current = list->first;
	int i = 0;
	while(current != NULL) {
		listPointer[i] = strdup(current->par->key);
		//verifica se o strdup() teve sucesso, se não liberta toda a memoria até ai reservada
		if(listPointer[i] == NULL) {
			int j;
			for(j = 0; j < i; j++) {
				free(listPointer[j]);
			}
			free(listPointer);
			return NULL;
		}

		current = current->next;
		i++;
	}
	listPointer[list->size] = NULL;
	return listPointer;

}

/* Devolve um array de entry * com a cópia de todas as entries da 
 * tabela, e um último elemento a NULL.
 */
struct entry_t **list_get_entries(struct list_t *list) {
	//Verificação dos parâmetros fornecidos
	if(list == NULL )
		return NULL;

	//Alocação de memória para a lista de apontadores
	struct entry_t **listEntries = (struct entry_t **) malloc (sizeof (struct entry_t*) * (list->size + 1));

	//Verificação da integridade da alocação da lista de apontadores
	if(listEntries == NULL)
		return NULL;

	struct node_t *current = list->first;
	int i = 0;
	while(current != NULL) {
		listEntries[i] = entry_dup(current->par);
		//verifica se o strdup() teve sucesso, se não liberta toda a memoria até ai reservada
		if(listEntries[i] == NULL) {
			int j;
			for(j = 0; j < i; j++) {
				entry_destroy(listEntries[j]);
			}
			free(listEntries);
			return NULL;
		}

		current = current->next;
		i++;
	}
	listEntries[list->size] = NULL;
	return listEntries;


}

/* Liberta a memoria reservada por list_get_keys.
 */
void list_free_keys(char **keys) {
	
	if(keys != NULL) {
		int i = 0;
		while(keys[i] != NULL) {
			free(keys[i]);
			i++;
		}
		free(keys);
	}
}

/* Liberta a memoria reservada por list_get_entries.
 */
void list_free_entries(struct entry_t **entries){

	if(entries != NULL) {
		int i = 0;
		while(entries[i] != NULL) {
			entry_destroy(entries[i]);
			i++;
		}
		free(entries);
	}

}



