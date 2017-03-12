/* Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include "data.h"
#include "list.h"
#include "entry.h"
#include "client_stub-private.h"
#include "client_stub.h"
#include "quorum_table.h"
#include "quorum_table-private.h"
#include "quorum_access.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Função para estabelecer uma associação entre uma tabela qtable_t e
 * um array de n servidores.
 * addresses_ports é um array de strings e n é o tamanho deste array.
 * Retorna NULL caso não consiga criar o qtable_t.
 */
struct qtable_t *qtable_bind(const char **addresses_ports, int n){

	 //verifica argumentos
    if(addresses_ports == NULL || n < 1)
        return NULL;

    //aloca memoria para estrutura
    struct qtable_t *qtable = (struct qtable_t*) malloc(sizeof(struct qtable_t));
    if(qtable == NULL) 
        return NULL;

    //cria array de rtables(vai guardar todos os sockets para ligar aos servidores)
    //struct rtable_t servers[n];
    struct rtable_t *servers = (struct rtable_t*) malloc (n * sizeof(struct rtable_t));
    qtable->rtables = servers;

    //preenche array de rtables
    int i, notNullEntries = 0;
    for(i = 0; i < n; i++) {

        struct rtable_t *temp = rtable_bind(addresses_ports[i]);

        if(temp != NULL){
            qtable->rtables[i] = *temp;
            notNullEntries++;
        } else {
            int j;
            for(j = 0; i < notNullEntries; j++) {
                rtable_unbind(&qtable->rtables[j]);
            }
            free(qtable);
            return NULL;
        }
    }

    //preenche campos restantes da estrutura qtable
    qtable->n_servers = n;


    int ret = init_quorum_access(qtable->rtables, qtable->n_servers);
    if(ret == -1) {
        qtable_disconnect(qtable);
        return NULL;
    }

    return qtable;

}


/* Fecha a ligação com os servidores do sistema e liberta a memória alocada
 * para qtable. 
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int qtable_disconnect(struct qtable_t *qtable) {

	if(qtable == NULL)
		return -1;
	int ret = destroy_quorum_access();
	free(qtable);

	return ret;
}

/* Função para adicionar um elemento na tabela.
 * Note que o timestamp em value será atribuído internamente a esta função,
 * como definido no algoritmo de escrita.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int qtable_put(struct qtable_t *qtable, char *key, struct data_t *value) {

	printf("qtable_put %s\n", key);

	if(qtable == NULL || key == NULL || value == NULL){
		printf("Parametros inválidos rtable_put\n");	
		return -1;
	}
	
	//DUP KEY
	char * duppedKey = strdup(key);
	if(duppedKey == NULL){
		printf("Erro alocação de memoria para a duplicação de chave\n");
		return -1;
	}
	
	//DUP DATA
	struct data_t *newData = (struct data_t *) malloc(sizeof(struct data_t));
	if(newData == NULL){
		printf("Erro alocação de memoria para a duplicação de data\n");
		free(duppedKey);
		return -1;
	}

	newData = qtable_get(qtable, duppedKey);
	if(newData != NULL){
		// this is a prove that the key is in the system
		// then you can not put it in the table
		return -1;
	}
	data_destroy(newData);

	newData = data_dup(value);
	newData->timestamp =  increment_ts(newData->timestamp, qtable->client_id);

	struct quorum_op_t *request = (struct quorum_op_t*) malloc(sizeof(struct quorum_op_t));
	if(request == NULL){
		return -1;
	}
	request->opcode = OC_PUT;
	request->content.entry = entry_create(key,newData);

	struct quorum_op_t **response = quorum_access(request, ceil2(qtable->n_servers)); 
	if(response == NULL){
			//destroy_uni_quorum(request);
		return -1;
	}
	int i;

	for (i = 0; i < qtable->n_servers; i++){
		// alguma das respostas veio com 0 logo um update foi feito
		if(response[i]!=NULL && response[i]->content.result==0){
			return 0;
		}
	}

 	return -1;
}

/* Função para atualizar o valor associado a uma chave.
 * Note que o timestamp em value será atribuído internamente a esta função,
 * como definido no algoritmo de update.
 * Devolve 0 (ok) ou -1 (problemas).
 */
int qtable_update(struct qtable_t *qtable, char *key, struct data_t *value){
	//Verifica parametros de entrada
	if(qtable == NULL || key == NULL || value == NULL){
		printf("Parametros inválidos rtable_upate\n");	
		return -1;
	}
	struct data_t *newData = qtable_get(qtable, key);
	if(newData == NULL){ // Não existe esta chave, logo retornar "não foi possivel".
		data_destroy(newData);
		return -1;
	}


	//existe a chave, logo incrementar timestamp e actulaliza-la
	struct data_t *actualData = data_dup(value);
	actualData->timestamp =  increment_ts(newData->timestamp, qtable->client_id);

	struct quorum_op_t * request = (struct quorum_op_t*) malloc(sizeof(struct quorum_op_t));
	if(request == NULL){
		data_destroy(newData);
		data_destroy(actualData);
		return -1;
	}

	request->opcode = OC_UPDATE;
	request->content.entry = entry_create(key,actualData);

	struct quorum_op_t **response = quorum_access(request, ceil2(qtable->n_servers));
	int i;

	for (i = 0; i < qtable->n_servers; i++){
		// alguma das respostas veio com 0 logo um update foi feito
		if(response[i]!=NULL && response[i]->content.result==0){
			return 0;
		}
	}
	return -1;

}

/* Função para obter um elemento da tabela.
 * Em caso de erro ou elemento não existente, devolve NULL.
 */
struct data_t *qtable_get(struct qtable_t *qtable, char *key) {
	if(qtable == NULL || key == NULL)
		return NULL;
	struct quorum_op_t *request = (struct quorum_op_t*) malloc(sizeof(struct quorum_op_t));
	if(request == NULL){
		return NULL;
	}
	request->opcode = OC_GET;
	request->content.key = strdup(key);


	struct quorum_op_t **response = quorum_access(request, ceil2(qtable->n_servers)); 
	if(response == NULL){
			//destroy_uni_quorum(request);
		return NULL;
	}
	

	long  long temp = 0;
	int indiceDataMaxTs;
	int i;
	for(i = 0; i<qtable->n_servers; i++){

		if(response[i] != NULL){
			if(temp < response[i]->content.data->timestamp){
				temp = response[i]->content.data->timestamp;
				indiceDataMaxTs = i;
			}
		}
	}

	struct data_t *newData = (struct data_t*) malloc(sizeof(struct data_t));
	if(newData == NULL){
		return NULL;
	}
	// biggest timestamp of data.
	newData = data_dup(response[indiceDataMaxTs]->content.data);

	if(newData->datasize == 0){
		free(newData);
		return NULL;
	}
	

	int j;
	for(j = 0; j < qtable->n_servers; j++){
	
		if(response[j] != NULL && j != indiceDataMaxTs && response[j]->content.data->timestamp != temp) {
			
			if(response[j]->content.data->datasize == 0 && response[j]->content.data->timestamp == 1){
				request->opcode = OC_PUT;
				request->content.entry = entry_create(key, newData);
				writeback(request, i);
			}else
			{
				request->opcode = OC_UPDATE;
				request->content.entry = entry_create(key, newData);
				writeback(request, i);
			}
		}
	}
	
	
	return newData;
}

/* Função para remover um elemento da tabela. É equivalente à execução 
 * put(k, NULL) se a chave existir. Se a chave não existir, nada acontece.
 * Devolve 0 (ok), -1 (chave não encontrada).
 */
int qtable_del(struct qtable_t *qtable, char *key) {
	struct data_t * d = data_create(0);
	int ret = qtable_update(qtable, key, d );
	data_destroy(d);
	return ret;
}

/* Devolve número (aproximado) de elementos da tabela ou -1 em caso de
 * erro.
 */
int qtable_size(struct qtable_t *qtable) {
	/* idealmente:
		criar opocode: GET_CHANGES 70.
		criar função qtable_get_num_change_ops()
		prencher quorum_op_t com {GET_CHANGES} e chamar quorum_acess
		Ver no array de respostas resposta mais alta
		fazer a operação size
		responder ao cliente com size eproveniente do servidor com maior numero de ops
	*/

	if(qtable == NULL)
		return -1;
	
	struct quorum_op_t *request = (struct quorum_op_t*) malloc(sizeof(struct quorum_op_t));
	if(request == NULL){
		return -1; // atenção que esta -1  não referente ah chave nao encontrada
	}

	request->opcode = OC_NUM_OPS;
	struct quorum_op_t **response = quorum_access(request, ceil2(qtable->n_servers)); 
	if(response == NULL){
		return -1;
	}

	int i;
	int indiceDataMaxChg;
	int maxChanges = 0;
	for(i=0; i<qtable->n_servers; i++){
		if(response[i]!= NULL){
			if(maxChanges < response[i]->content.result){
				maxChanges= response[i]->content.result;
				indiceDataMaxChg = i;
			}		
		}		
	}

	request->opcode = OC_SIZE;
	struct quorum_op_t **response2 = quorum_access(request, ceil2(qtable->n_servers));
	if(response == NULL){
		free(response);
		return -1;
	}

	int result = 0;
	if(response2[indiceDataMaxChg] != NULL)
		result = response2[indiceDataMaxChg]->content.result;
	
	else {

		for(i=0; i<qtable->n_servers; i++){
		if(response2[i]!= NULL){
			result += response2[i]->content.result;
			}		
		}
		result = (int) result/ceil2(qtable->n_servers);		
	}

	return result;

}

/* Devolve um array de char* com a cópia de todas as keys da tabela,
 * e um último elemento a NULL. Esta função não deve retornar as
 * chaves removidas, i.e., a NULL.
 */
char **qtable_get_keys(struct qtable_t *qtable) {
	if(qtable == NULL)
		return NULL;
	
	struct quorum_op_t *request = (struct quorum_op_t*) malloc(sizeof(struct quorum_op_t));
	if(request == NULL){
		return NULL; 
	}

	request->opcode = OC_NUM_OPS;
	struct quorum_op_t **response = quorum_access(request, ceil2(qtable->n_servers)); 
	if(response == NULL){
		return NULL;
	}

	int i;
	int indiceDataMaxChg;
	int maxChanges = 0;
	for(i=0; i<qtable->n_servers; i++){
		if(response[i]!= NULL){
			if(maxChanges < response[i]->content.result){
				maxChanges= response[i]->content.result;
				indiceDataMaxChg = i;
			}		
		}		
	}

	request->opcode = OC_GET;
	request->content.key = strdup("!");
	struct quorum_op_t **response2 = quorum_access(request, ceil2(qtable->n_servers));
	if(response == NULL){
		free(response);
		return NULL;
	}


	if(response2[indiceDataMaxChg] != NULL)
		return response2[indiceDataMaxChg]->content.keys;

	return NULL;

}

/* Liberta a memória alocada por qtable_get_keys().
 */
void qtable_free_keys(char **keys) {
	if(keys != NULL)
		list_free_keys(keys);
}

/*Incrementa o valor de um timestamp dado um cliente id
 *Usa o algoritmo do professor
*/
long long increment_ts( long long ts, int id) {

	long newts = (long) ts/1000;
	newts += 1;
	long long ret = newts * 1000;
	return ret + id;
}

int ceil2(int n){
	return ceil((n+1)/2);
}

void destroy_uni_quorum(struct quorum_op_t *q){
	if(q != NULL){
		if(q->content.entry != NULL)		
			entry_destroy(q->content.entry);
		if(q->content.key != NULL)
			free(q->content.key);
		if(q->content.keys != NULL);
			qtable_free_keys(q->content.keys);
		if(q->content.data !=NULL)
			data_destroy(q->content.data);

		free(q);
	}
}

void destroy_mult_quoruns(struct quorum_op_t **qq, int n){
	int i;
	for(i=0; i<n; i++){
		destroy_uni_quorum(qq[i]);
	}
	free(qq);
}
