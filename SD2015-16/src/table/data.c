/* PROJECTO 1 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include <stdlib.h>
#include <string.h>
#include "data.h"

/* Função que cria um novo elemento de dados data_t e reserva a memória
 * necessária, especificada pelo parâmetro size 
 */
struct data_t *data_create(int size) {

	//Verificação dos argumentos fornecidos
	if(size < 0) 
		return NULL;
	
	//Alocação de memória para a estrutura data_t
	struct data_t *newData = (struct data_t *) malloc (sizeof(struct data_t));

	//Verificação da integridade da alocação de newData
	if(newData == NULL)
		return NULL;

	newData->datasize = size;
	newData->timestamp = 1;
	
	if(size == 0) 
		newData->data = NULL;

	else {
		newData->data = malloc (size);

		//Verificação da integridade da alocação de newData->data
		if(newData->data == NULL) {
			free(newData);
			return NULL;
		}
	}

	return newData;

}

/* Função idêntica à anterior, mas que inicializa os dados de acordo com
 * o parâmetro data.
 */
struct data_t *data_create2(int size, void * data) {

	//Verificação dos argumentos
	if(size < 0)
		return NULL;
	if(size > 0 && data == NULL)
		return NULL;
	
	if(size == 0) {
		return data_create(0);
	}

	struct data_t *newData = data_create(size);

	//Verificação da integridade da alocação de newData
	if(newData == NULL)
		return NULL;

	memcpy(newData->data, data, size);

	//Verificação da integridade da cópia para newData->data
 	if(newData->data == NULL){
		data_destroy(newData);
		return NULL;
	}

	return newData;

} 

/* Função que destrói um bloco de dados e liberta toda a memória.
 */
void data_destroy(struct data_t *data) {

	if(data != NULL) {
		free(data->data);
		free(data); 
	}
}

/* Função que duplica uma estrutura data_t.
 */
struct data_t *data_dup(struct data_t *data) {

	if(data == NULL)
		return NULL;
	
	//Verificações feitas anteriormente
	struct data_t *dataDup = data_create2(data->datasize, data->data);
	if(dataDup != NULL)
		dataDup->timestamp = data->timestamp;
	
	return dataDup;
}
