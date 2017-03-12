/* PROJECTO 3 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#ifndef _NETWORK_SERVER_H
#define _NETWORK_SERVER_H

#include "table.h"
#include "message.h"


/*Cria um socket para comunicação com o exterior*/
/*Devolve o fd do scket criado ou -1 em caso de erro*/
int create_connection( int porto );

/*Preenche uma estrutura message_t de erro a enviar ao cliente*/
struct message_t *errorStruct(struct message_t *erro);

/* Executa o pedido enviado pelo cliente e retorna um estrutura message_t com a resposta*/
struct message_t *executa_e_responde(struct table_t *table, struct message_t *message);

/*Trata dos pedidos do cliente, isto é recebe uma mensagem e:
 *desserializa-a
 *executa o pedido
 *cria mensagem de resposta
 *serializa a mensagem
 *envia a mensagem para o cliente
 *Retorna 0 em caso de sucesso, e -1 em caso de erro ou terminaçao de ligaçao
 */
int network_receive_send(int sockFD, struct table_t *table );

#endif