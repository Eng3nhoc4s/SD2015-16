/* PROJECTO 2 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "entry.h"
#include "list.h"
#include "table-private.h"


/*funcao de hash: a partir da chave devolve um indice na tabela 
*/ 
int hash (char *key, int n) {
	int soma = 0;
	int i = 0;
	int len = strlen(key);
	
	if(len <= 6) {
		while(key[i] != '\0') {
			soma = soma + (int) key[i];
			i++;
		}
	}
	else
		while(i < 3) {
			soma = soma + (int) key[i] + (int) key[len - i];
			i++;
		}

	return soma % n;	
}

 /* Função para criar/inicializar uma nova tabela hash, com n  
 * linhas(n = módulo da função hash)
 */
struct table_t *table_create(int n) {
	//verifica parametro de entrada
	if(n <= 0)
		return NULL;

	//aloca memoria para a tabela
	struct table_t *table = (struct table_t* ) malloc(sizeof(struct table_t));
	if(table == NULL)
		return NULL;

	//aloca memoria para o array de listas
	table->table = (struct list_t**) malloc (sizeof(struct list_t *) * n);
	if(table->table == NULL) {
		free(table);
		return NULL;
	}

	//cria uma lista para cada posição da tabela
	int i;
	for(i = 0; i < n; i++) {
		table->table[i] = list_create();
		//verifica criação e destroi tudo caso haja alguma posição que nao tenha sido criada
		if(table->table[i] == NULL) {
			int j;
			for(j=0; j < i; j++)
				list_destroy(table->table[j]);
			free(table->table);
			free(table);
			return NULL;
		}
	}

	table->capacity = n;
	table->size = 0;
	table->nChanges = 0;

	return table;
}

/* Libertar toda a memória ocupada por uma tabela.
 */
void table_destroy(struct table_t *table) {
	if(table != NULL) {
		int i;
		for(i = 0; i < table->capacity; i++)
			list_destroy(table->table[i]);

		free(table->table);
		free(table);
	}
}

/* Função para adicionar um par chave-valor na tabela. 
 * Os dados de entrada desta função deverão ser copiados.
 * Devolve 0 (ok) ou -1 (out of memory, outros erros)
 */
int table_put(struct table_t *table, char *key, struct data_t *value) {
	//verifica parametros de entrada
	if(table == NULL || key == NULL || value == NULL)
		return -1;

	//cria um novo entry(chave-valor)
	struct entry_t *newEntry = entry_create(key,value);
	if(newEntry == NULL) 
		return -1;

	//devolve um indice no array de list
	int index = hash(key,table->capacity);

	//adiciona a entry na tabela
	int ret = list_add(table->table[index], newEntry);
	table->size = table->size + (ret + 1);
	entry_destroy(newEntry);


	table->nChanges++;
	printf("PUT: %s\n", key);
	return ret;
}

/* Função para substituir na tabela, o valor associado à chave key. 
 * Os dados de entrada desta função deverão ser copiados.
 * Devolve 0 (OK) ou -1 (out of memory, outros erros)
 */
int table_update(struct table_t *table, char *key, struct data_t *value) {
	//verifica parametros de entrada
	if(table == NULL || key == NULL || value == NULL)
		return -1;

	//devolve um indice no array de list
	int index = hash(key,table->capacity);

	//obtem apontador para a entry pretendida
	struct entry_t *entry = list_get(table->table[index],key);
	if(entry == NULL) 
		return -1;

	/*
	//***Verifica se entry->value->datasize == 0 ( significa que foi apagado,logo nao mexer)
	if(entry->value->datasize == 0)
		return -1;
	*/

	//****verifica se timestamp de value é inferior ao do que já la está( nesse caso nao actualiza) 
	if( ((long)(entry->value->timestamp/1000)) >= ((long) (value->timestamp)) )
		return -1;

	//duplica o valor do data
	struct data_t *newData = data_dup(value);
	if(value == NULL)
		return -1;

	//Destroi data anterior e substitui-a pelo novo data.
	data_destroy(entry->value);
	entry->value = newData;

	//*** Caso a nova data seja null(simula um del, logo diminuir size)
	if(newData->datasize == 0)
		table->size--;
	
	
	table->nChanges++;
	printf("UPDATE: %s\n", key);
	return 0;
}

/* Função para obter da tabela o valor associado à chave key.
 * A função deve devolver uma cópia dos dados que terão de ser libertados
 * no contexto da função que chamou table_get.
 * Devolve NULL em caso de erro.
 */
struct data_t *table_get(struct table_t *table, char *key) {
	//verifica parametros de entrada
	if(table == NULL || key == NULL)
		return NULL;

	//devolve um indice no array de list
	int index = hash(key,table->capacity);

	//Obtem apontador para a entry pretendida
	struct entry_t *entry = list_get(table->table[index],key);
	if(entry == NULL) 
		return data_create(0);

	printf("GET: %s\n", key);
	return data_dup(entry->value);
}

/* Função para remover um par chave valor da tabela, especificado 
 * pela chave key, libertando a memória associada a esse par.
 * Devolve: 0 (OK), -1 (nenhum tuplo encontrado; outros erros)
 */
int table_del(struct table_t *table, char *key) {
	//verifica parametros de entrada
	if(table == NULL || key == NULL)
		return -1;

	//devolve um indice no array de list
	int index = hash(key,table->capacity);

	//remove o elemento de chave key da tabela
	int ret = list_remove(table->table[index], key);
	//***table->size = table->size - (1 + ret);

	
	table->nChanges++;
	printf("DEL: %s\n", key);
	return ret;
}

/* Devolve o número de elementos na tabela.
 */
int table_size(struct table_t *table) {
	printf("SIZE\n");
	return table == NULL ? -1 : table->size;
}

/* Devolve um array de char * com a cópia de todas as keys da tabela,
 * e um último elemento a NULL.
 */
char **table_get_keys(struct table_t *table) {
	//verificaçao dos parametros de entrada
	if(table == NULL)
		return NULL;

	//Alocaçao de memoria para um array de Strings 
	char **arrayPointer = (char **) malloc (sizeof (char*) * (table->size + 1));
	if(arrayPointer == NULL)
		return NULL;
	//table Index
	int tI = 0;
	//arrayPointer Index
	int aPI =0;

	while(aPI < table->size) {
		
		char **aux = list_get_keys(table->table[tI]);
		if(aux != NULL) {
			//aux Index
			int xI = 0;
			while(aux[xI] != NULL) {
				arrayPointer[aPI] = aux[xI];
				aPI++;
				xI++;
			}
			free(aux);
		}//Caso List_get_key devolva NULL, limpar a memoria ate ai reservada no arrayPointer
		else{
			int i;
			for(i=0; i < aPI++; i++)
				free(arrayPointer[i]);
			free(arrayPointer);
			return NULL; 
		}

		tI++;
	}

	//coloca o último elemento a NULL
	arrayPointer[table->size] = NULL;
	
	return arrayPointer;
}

/* Liberta a memória alocada por table_get_keys().
 */
void table_free_keys(char **keys) {
	list_free_keys(keys);
}

/* Retorna o número de alterações realizadas na tabela.
*/
int table_get_num_change_ops(struct table_t *table){
	printf("NUN_CHANGES:%d\n ", table->nChanges);
	return table == NULL ? -1 : table->nChanges;
}
/* Devolve um array de entry_t* com cópias de todas as entries
* da tabela, e um último elemento a NULL.
*/
struct entry_t **table_get_entries(struct table_t *table){
	if(table == NULL){
		return NULL;
	}
	struct entry_t **arrayEntries = (struct entry_t **) malloc (sizeof(struct entry_t*) * (table->size + 1));
	if(arrayEntries == NULL){
		return NULL;
	}
	int k = 0;
    int i;
    for (i = 0; i < table->capacity; i++){
        char **tempListKeys = list_get_keys(table->table[i]);
        int listSize = list_size(table->table[i]); 
        if(tempListKeys != NULL){
            int j;
            for(j=0; j< listSize; j++){
				struct entry_t *copyEntry = entry_dup(list_get(table->table[i], tempListKeys[j]));
				if(copyEntry == NULL){
					// pensar nisto
				}

				//***para nao adicionar chaves com data a null(entradas que foram apagadas)
				if(copyEntry->value->datasize != 0){
                	arrayEntries[k] = copyEntry;
                	k++;
                }
                else
                	entry_destroy(copyEntry);
            }
            free(tempListKeys);
        }
 
    }
    arrayEntries[table->size] = NULL;
    return arrayEntries;
}
/* Liberta a memória alocada por table_get_entries().
*/
void table_free_entries(struct entry_t **entries){
	if(entries != NULL){
		int i = 0;
			while(entries[i] != NULL){
				entry_destroy(entries[i]);
				i++;
			}
		free(entries);
	}
}

/* Função para obter o timestamp do valor associado a uma chave.
* Em caso de erro devolve -1. Em caso de chave não encontrada devolve 0.
*/
long long table_get_ts(struct table_t *table, char *key) {
	struct data_t *data = table_get(table, key);
	if(data == NULL) {
		printf("GET TIMESTAMP -1: %s\n", key);
		return -1;
	}

	//Chave não encontrada(neste caso significa que o size do data == 0)
	if(data->datasize == 0) {
		data_destroy(data);
		printf("GET TIMESTAMP 0: %s\n", key);
		return 0;
	}

	long long timestamp = data->timestamp;
	data_destroy(data);
	printf("GET TIMESTAMP: %s %lld\n", key, timestamp);
	return timestamp; 
}







