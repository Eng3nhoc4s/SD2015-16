/* Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */
#include "quorum_access-private.h"
#include "quorum_access.h"
#include "client_stub.h"
#include "network_client-private.h"
#include "queue.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

/*variaveis globais que guardam uma series de dados*/

int *quorum;   //array que guarda os servidores num quorum
struct thread_data *threads; //contem informaçao para cada thread
struct queue_t *queues; // Filas das threads e main para envio e receçao de respostas
struct rtable_t *rtable; // rtable para estabelecimento de ligaçao com servidor
pthread_t *thread_ids; //Guarda os identificadores das threads criadas
int n_servers; // numero de servidores total
short forma_quorum; // flag para indicar se é necessario formar quorum
int id_counter; // id unico da operação
int pointer_quorum; // apontador para o array de quorum para saber quais as trocas que houve

 /* Esta função deve criar as threads e as filas de comunicação para que o
 * cliente invoque operações a um conjunto de tabelas em servidores
 * remotos. Recebe como parâmetro um array rtable de tamanho n.
 * Retorna 0 (OK) ou -1 (erro).
 */
int init_quorum_access(struct rtable_t *rtable2, int n) {

	printf("NUMERO DE SERVERS = %d\n", n);
	
	 

	threads = (struct thread_data*) malloc(sizeof(struct thread_data) * n);
	if(threads == NULL) {
		return -1;
	}
	queues = (struct queue_t*) malloc (sizeof(struct queue_t) * (n+1));
	if(queues == NULL) {
		free(threads);
		return -1;
	}
	rtable = (struct rtable_t*) malloc (sizeof(struct rtable_t) * n);
	if(rtable == NULL) {
		free(queues);
		free(threads);
		return -1;
	}
	thread_ids = (pthread_t *) malloc (sizeof(pthread_t ) * n);
	if(thread_ids == NULL) {
		free(rtable);
		free(queues);
		free(threads);
		return -1;
	}
	
	n_servers = n;

	if (init_task_queues(n + 1, queues) != 0){ /* Cria array de filas */
		perror("Impossível criar filas.");
		free(rtable);
		free(queues);
		free(threads);
		return -1;
	}

	int i;

	//declaracao de main queue com sua flag
	queues[n].is_main = 1;

	int ceiling =(int) ceil((n+1)/2);
	// iniciação do quorum
	quorum = (int *) malloc (sizeof(int) * n);

	// Criar THREADS e passar-lhes parâmetros
	for(i = 0; i < n; i++){
		//transferir rtable do argumento da função para variavel global
		rtable[i] = rtable2[i];

		//preenche estrutura threads_data
		threads[i].my_queue = &queues[i]; // Para a thread ler tarefas
		threads[i].main_queue = &queues[n]; // Para a thread colocar resposta a tarefas
		threads[i].my_id = i; // id da thread
		threads[i].rtable = &rtable[i];
		threads[i].position = i;

		// Criar cada uma das threads
		if (pthread_create(&thread_ids[i], NULL, &server_access, (void *) &threads[i]) != 0){
			perror("Erro ao criar uma thread.");
			int j;
			for(j = 0; j < i; j++) {
				pthread_cancel(thread_ids[j]);
			}
			destroy_task_queues(n, queues);
			free(rtable);
			free(queues);
			free(threads);
			return -1;			    
		}
		pthread_detach(thread_ids[i]);
		quorum[i] = i;
	}



	forma_quorum = 1;
	id_counter = 1;
	pointer_quorum = ceiling;
	return 0;

}


/* Função que envia um pedido a um conjunto de servidores e devolve
 * um array com o número esperado de respostas.
 * O parâmetro request é uma representação do pedido, enquanto
 * expected_replies representa a quantidade de respostas esperadas antes 
 * da função retornar.
 * Note que os campos id e sender em request serão preenchidos dentro da
 * função. O array retornado é um array com k posições (0 a k-1), sendo cada
 * posição correspondente a um apontador para a resposta de um servidor 
 * pertencente ao quórum que foi contactado.
 * Caso não se consigam respostas do quórum mínimo, deve-se retornar NULL.
 */
struct quorum_op_t **quorum_access(struct quorum_op_t *request, 
                                   int expected_replies) {

	int i;
	//Na primeira vez e qd necessario, forma um quorum de servidores
	if(forma_quorum) {
		int ret = formar_quorum( expected_replies);
		if(ret == -1){
			return NULL;
		}
		forma_quorum = 0;
	}
	
	request->id = id_counter;
	

	for(i=0; i < expected_replies; i++) {
		int server = quorum[i];
		add_queue(&queues[server], request);
	}	

	struct quorum_op_t **answers = (struct quorum_op_t **) malloc(sizeof(struct quorum_op_t*)*n_servers);
	if(answers == NULL) {
		return NULL;
	}
	for(i = 0; i < n_servers; i++)
		answers[i] = NULL;

	int respostas_recebidas = 0;
	struct quorum_op_t *res = (struct quorum_op_t*) malloc(sizeof(struct quorum_op_t));

	while(respostas_recebidas < expected_replies) {
		res = remove_queue(&queues[n_servers]);

		while(res == NULL) {
			add_queue(&queues[quorum[pointer_quorum]],request);
			res = remove_queue(&queues[n_servers]);
			if(res == NULL)
				pointer_quorum++;
			if(pointer_quorum == n_servers)
				return NULL;
			forma_quorum = 1;		
		}
		
		if(res->id == id_counter) {
			answers[res->sender] = res;
			respostas_recebidas++;
		}
	}
	id_counter ++;
	return answers;
}

/* Liberta a memoria, destroi as rtables usadas e destroi as threads.
 */
int destroy_quorum_access() {
	int i;
	for(i = 0; i < n_servers; i++ ) {
		pthread_cancel(thread_ids[i]);
	}

	destroy_task_queues(n_servers, queues);
	free(rtable);
	free(queues);
	free(threads);
	free(quorum);
	return 0;
}


/* Função que vai ser executada por cada uma das threads
 */
void *server_access(void *p) {

	int fatal_error = 0;
	struct thread_data *my_parameters = p;

	printf("ENTREI server access\n");
	
	
	printf("Sou a thread %d\n", my_parameters->my_id);
  
	while(!fatal_error){
	
		// Ler uma tarefa da QUEUE
		struct quorum_op_t *tarefa = remove_queue(my_parameters->my_queue);

		printf("  Thread %d, recebeu tarefa com opcode %d\n ", my_parameters->my_id, tarefa->opcode);

		struct quorum_op_t *resposta;

		switch(tarefa->opcode) {

			case OC_PUT:
				printf("Entrou no case PUT: %s\n", tarefa->content.entry->key);
				resposta = (struct quorum_op_t *) malloc(sizeof(struct quorum_op_t));
				resposta->id = tarefa->id;
				resposta->sender = my_parameters->my_id;
				resposta->opcode = tarefa->opcode +1;
				resposta->content.result = rtable_put(my_parameters->rtable, tarefa->content.entry->key, tarefa->content.entry->value );
				break;

			case OC_GET:

				resposta = (struct quorum_op_t *) malloc(sizeof(struct quorum_op_t));
				resposta->id = tarefa->id;
				resposta->sender = my_parameters->my_id;
				resposta->opcode = tarefa->opcode +1;

				if(strcmp(tarefa->content.key, "!") == 0) {
					char ** keys = rtable_get_keys(my_parameters->rtable);
					resposta->content.keys = keys;
				}
				else {
					struct data_t *toReturn = rtable_get(my_parameters->rtable, tarefa->content.key);
					resposta->content.data = toReturn;
				}
				break;

			case OC_UPDATE:

				resposta = (struct quorum_op_t *) malloc(sizeof(struct quorum_op_t));
				resposta->id = tarefa->id;
				resposta->sender = my_parameters->my_id;
				resposta->opcode = tarefa->opcode +1;
				resposta->content.result = rable_update(my_parameters->rtable, tarefa->content.entry->key, tarefa->content.entry->value);
				break;

			case OC_RT_GETTS:
				printf("Entrou no case GET_TS: %s\n", tarefa->content.key);
				resposta = (struct quorum_op_t *) malloc(sizeof(struct quorum_op_t));
				resposta->id = tarefa->id;
				resposta->sender = my_parameters->my_id;
				resposta->opcode = tarefa->opcode +1;
				resposta->content.timestamp =rtable_get_ts(my_parameters->rtable, tarefa->content.key);
				printf(">>>>>>>>>>>>>>>TIMESTAMP resposta %lld\n",resposta->content.timestamp);
				break;

			case OC_SIZE:

				resposta = (struct quorum_op_t *) malloc(sizeof(struct quorum_op_t));
				resposta->id = tarefa->id;
				resposta->sender = my_parameters->my_id;
				resposta->opcode = tarefa->opcode +1;
				resposta->content.result = rtable_size(my_parameters->rtable);
				printf("**********Entrou no case SIZE: %d\n", resposta->content.result);
				break;

			case OC_NUM_OPS:

				resposta = (struct quorum_op_t *) malloc(sizeof(struct quorum_op_t));
				resposta->id = tarefa->id;
				resposta->sender = my_parameters->my_id;
				resposta->opcode = tarefa->opcode +1;
				resposta->content.result = rtable_num_ops(my_parameters->rtable);
				printf("**********Entrou no case NUM OPS: %d\n", resposta->content.result);
				break;

		}

		// Colocar na fila da thread principal
		add_queue(my_parameters->main_queue, resposta);
	}
	return NULL;
}

/*Função que atualiza o estado de uma tabela
 *Recebe um quorum_op_t request e um numero de uma fila
 *Envia para o servidor x o pedido request
 */
void writeback(struct quorum_op_t *request, int x) {
	printf("WriteBACK ON\n");
	request->id = id_counter;
	add_queue(&queues[x], request);
	id_counter++;
}

int formar_quorum( int expected_replies) {
	struct quorum_op_t *size = (struct quorum_op_t*) malloc (sizeof(struct quorum_op_t));
		size->opcode = OC_SIZE;
		size->id = -1;
		printf("quorum_access quorum_op_t size = %d\n", size->id);
		int i;
		for(i = 0; i <n_servers; i++) {
			add_queue(&queues[i], size);
		}

		printf("Cheguei aqui, quorum_access ->formar quorum\n");
		int counter = 0;
		for(i=0; i < n_servers; i++) {
			struct quorum_op_t *sizeRps;
			sizeRps = remove_queue(&queues[n_servers]);
			if(sizeRps == NULL) {
				continue;
			}
			if(sizeRps->id == -1) {
				int sender = sizeRps->sender;
				swap(sender, i);
				counter++;
			}
			if(counter == expected_replies){
		
				//free(size);
				return 0;
			}
		}
		
	free(size);

	return -1;
}

void swap(int sender, int indice) {
	int positionSender = threads[sender].position;
	int positionIndice = quorum[indice];
	quorum[indice] = sender;
	quorum[positionSender] = positionIndice;
	threads[sender].position = indice;
	threads[indice].position = positionSender; 
}



