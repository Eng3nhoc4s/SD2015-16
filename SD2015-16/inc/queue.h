/* Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */
#ifndef _QUEUE_H
#define _QUEUE_H

#include "quorum_access.h"
#include <errno.h>
struct qnode_t {     /* Nó de uma fila de tarefas */
	struct quorum_op_t *task;
	struct qnode_t *next;
};

struct queue_t {
	int is_main;
	struct qnode_t *queue_head;
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_not_empty;
};



/* Inicialização de um array com n filas de tarefas.
 */
int init_task_queues(int n, struct queue_t *nqueues);

/* Libertar memória relativa a n filas de tarefas.
 */
void destroy_task_queues(int n, struct queue_t *nqueues);

/* Inserir uma tarefa numa fila.
 */
int add_queue(struct queue_t *queue, struct quorum_op_t *task);

/* Retirar uma tarefa da fila
 */
struct quorum_op_t *remove_queue(struct queue_t *queue);

#endif
