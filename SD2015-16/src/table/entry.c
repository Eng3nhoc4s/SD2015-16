/* PROJECTO 1 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include <stdlib.h>
#include <string.h>
#include "entry.h"
#include "data.h"

/* Função que cria um novo par {chave, valor} (isto é, que inicializa
 * a estrutura e aloca a memória necessária).
 */
struct entry_t *entry_create(char* key, struct data_t *data) {
	
	//Verificação dos parâmetros fornecidos
	if(key == NULL || data == NULL)
		return NULL;

	//Alocação de  memoria para a estrutura entry_t
	struct entry_t *newEntry = (struct entry_t *) malloc (sizeof(struct entry_t));

	//Verificação da integridade da alocação de newEntry
	if(newEntry == NULL)
		return NULL;

	//Alocação de memoria e cópia da string fornecida para a entry
	newEntry->key = strdup(key);

	//Verificação da integridade da cópia da key
	if(newEntry->key == NULL) {
		free(newEntry);
		return NULL;
	}

	//Duplica a estrutura data_t
	newEntry->value = data_dup(data);
	
	//Verificação da integridade da duplicação de data
	if(newEntry->value == NULL) {
		free(newEntry->key);
		free(newEntry);
		return NULL;
	}

	return newEntry;
}

/* Função que destrói um par {chave-valor} e liberta toda a memória.
 */
void entry_destroy(struct entry_t *entry) {

	if(entry != NULL) {
		free(entry->key);
		data_destroy(entry->value);
		free(entry); 
	}
}

/* Função que duplica um par {chave, valor}.
 */
struct entry_t *entry_dup(struct entry_t *entry) {
	
	//Verificação dos parâmetros fornecidos
	if(entry == NULL)
		return NULL;
	
	struct entry_t *entryDup = entry_create(entry->key, entry->value);

	return entryDup;

}
