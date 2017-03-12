/* Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "queue.h"

 /* Inicialização de um array com n filas de tarefas.
 */
int init_task_queues(int n, struct queue_t *nqueues){
	int i;
	int j;

	for (i = 0; i < n; i++){
		nqueues[i].queue_head = NULL;
		if (pthread_mutex_init(&nqueues[i].queue_lock, NULL) != 0){
			for(j = 0; j < i; j++){
				pthread_mutex_destroy(&nqueues[j].queue_lock);
				pthread_cond_destroy(&nqueues[j].queue_not_empty);
			}
			return -1;
		}
		if (pthread_cond_init(&nqueues[i].queue_not_empty, NULL) != 0){
			for(j = 0; j < i; j++){
				pthread_mutex_destroy(&nqueues[j].queue_lock);
				pthread_cond_destroy(&nqueues[j].queue_not_empty);
			}
			pthread_mutex_destroy(&nqueues[i].queue_lock);
			return -1;
		}
	}

	return 0;
}

/* Libertar memória relativa a n filas de tarefas.
 */
void destroy_task_queues(int n, struct queue_t *nqueues){
	int i;
	struct qnode_t *aux;

	for (i = 0; i < n; i++){ /* Libertar nós em cada fila */
		while (nqueues[i].queue_head != NULL){
			aux = nqueues[i].queue_head;
			nqueues[i].queue_head = nqueues[i].queue_head->next;
			free(aux);
		}
		pthread_mutex_destroy(&nqueues[i].queue_lock);
		pthread_cond_destroy(&nqueues[i].queue_not_empty);
	}	
}

/* Inserir uma tarefa numa fila.
 */
int add_queue(struct queue_t *queue, struct quorum_op_t *task) {


	struct qnode_t *qnode;
	struct qnode_t *aux;

	if (pthread_mutex_lock(&queue->queue_lock) != 0) /* Obter o MUTEX */
		return -1;

	if ((qnode = (struct qnode_t *) malloc(sizeof(struct qnode_t))) == NULL) /* Criar um nó da fila */
		return -1;

	qnode->task = task; /* Inserir a tarefa */
	qnode->next = NULL;

	if (queue->queue_head == NULL) /* Colocar o nó no início da fila (cabeça da lista) */
		queue->queue_head = qnode;
	else{			       /* Colocar no fim da fila */
		aux = queue->queue_head;
		while (aux->next != NULL)
			aux = aux->next;
		aux->next = qnode;
	}

	pthread_cond_signal(&queue->queue_not_empty);
	pthread_mutex_unlock(&queue->queue_lock); /* Libertar o MUTEX */

	printf("ADD_queue %d\n", task->opcode);
	return 0;
}

/* Retirar uma tarefa da fila
 */
struct quorum_op_t *remove_queue(struct queue_t *queue) {

	if(queue == NULL)
		printf("NULL\n");

	struct qnode_t *aux;
	struct quorum_op_t *rtask;
		int ret;
	if ((ret = pthread_mutex_lock(&queue->queue_lock))!= 0) {
		return NULL;
	}
	if( queue->is_main == 1 )
	{
		struct timeval now;
		struct timespec timeToWait;

		//receber tempo actual
		gettimeofday(&now, NULL);

		//tempo de espera
		timeToWait.tv_sec = now.tv_sec;
		timeToWait.tv_nsec = now.tv_usec * 1000;
		timeToWait.tv_sec += 5;

		int wait;
		while(queue->queue_head == NULL && wait != ETIMEDOUT)
			wait = pthread_cond_timedwait(&queue->queue_not_empty, &queue->queue_lock, &timeToWait); /* Espera que exista uma tarefa */

		if( wait == ETIMEDOUT )
		{
			pthread_mutex_unlock(&queue->queue_lock);
			printf("TIMEOUT UNLOCKED====================\n");
			return NULL;
		}
		printf("MAIN REMOVE::::::::::::");
		
	}
	else
	{
		while(queue->queue_head == NULL)
			pthread_cond_wait(&queue->queue_not_empty, &queue->queue_lock); /* Espera que exista uma tarefa */
	}
	aux = queue->queue_head;
	rtask = aux->task;
	queue->queue_head = aux->next; /* Retirar o nó da lista */ 
	//free(aux); 

	pthread_mutex_unlock(&queue->queue_lock); /* Libertar o MUTEX */
	printf("Remove rtask: %d\n", rtask->opcode);
	return rtask;	
}
