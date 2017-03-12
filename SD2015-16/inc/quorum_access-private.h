#ifndef _QUORUM_ACCESS_PRIVATE_H
#define _QUORUM_ACCESS_PRIVATE_H

//#include "queue.h"
#include <pthread.h>
#include <quorum_access.h>

/* Encapsula a informação necessária a cada thread */
struct thread_data{
  struct queue_t *my_queue;
  struct queue_t *main_queue;
  int my_id;
  int position; 
  struct rtable_t *rtable;
};


/* Função que vai ser executada por cada uma das threads
 */
void *server_access(void *p);

/*Função que atualiza o estado de uma tabela
 *Recebe um quorum_op_t request e um numero de uma fila
 *Envia para o servidor x o pedido request
 */
void writeback(struct quorum_op_t *request, int x);

/*Responsável por formar um quorum de k servidores
 *Envia um pedido Size para todos os servidores,
 * os primeiros K a responder farão parte do quorum.
 *Usa variaveis globais no ficheiro quorum acess.
 *Retorna -1 caso nao coniga formar quorum e 0 caso contrário
 */
int formar_quorum(int k);

/*Troca 2 posicoes nun array de inteiros
 *Serve para saber quais os servidores que estão num quorum
 *Os servidores pertencentes ao quorum estão nas primeiras k posiçoes do array quorum
*/
void swap(int sender, int indice);



#endif
