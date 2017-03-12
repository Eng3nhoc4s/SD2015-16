/* PROJECTO 4 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include "persistent_table.h"
#include "table_skel-private.h"
#include "persistence_manager.h"
#include "client_stub-private.h"
#include "persistent_table-private.h"
#include "message-private.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Variavel global que guarda o apontador para a table, para a ptable e para o pmanager
struct  table_skel_t *tables;

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Recebe tambem uma string com o nome do ficheiro de Log.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists, char * filename) {

	//verifica argumentos de entrada
	if(n_lists < 1 || filename == NULL) {
		return -1;
	}

	//Aloca memoria para uma estrutura que alberga a variavel global
	tables = (struct table_skel_t*) malloc(sizeof(struct table_skel_t));
	if(tables == NULL) {
		return -1;
	}
	//inicilaiza uma tabela
	struct table_t *table = table_create(n_lists);
	if(table == NULL) {
		free(tables);
		return -1;
	}
	//guarda o endereço do pmanager.
	tables->pmanager = pmanager_create(filename, LOG_SIZE);
	if(tables->pmanager == NULL) {
		free(tables);
		return -1;
	}

	//guarda o endereço de uma ptable
	tables->persistent = ptable_open(table, tables->pmanager);
	if(tables->persistent == NULL) {
		free(tables->pmanager);
		free(tables);
		return -1;
	}

	return 0;

}

/* Libertar toda a memória e recursos alocados pela função anterior.
 */
int table_skel_destroy() {
	if(tables == NULL)
		return -1;

	if(tables->persistent != NULL)
		ptable_destroy(tables->persistent);

	free(tables);

	return 0;
}

/* Executa uma operação (indicada pelo opcode na msg_in) e retorna o
 * resultado numa mensagem de resposta ou NULL em caso de erro.
 */
struct message_t *invoke(struct message_t *message) {

	struct message_t *resposta = (struct message_t *) malloc (sizeof(struct message_t));
	if(resposta == NULL)
		return NULL;

	int ret;
	switch(message->opcode) {

		case OC_PUT:
			//Verifica coerencia entre codigo da operação e conteudo da mensagem
			if(message->c_type != CT_ENTRY) {
				return errorStruct(resposta);
			}

			//executa pedido
			ret = ptable_put(tables->persistent, message->content.entry->key, message->content.entry->value);
			if(ret < 0)
				return errorStruct(resposta);

			//Cria resposta
			resposta->opcode = OC_PUT + 1;
			resposta-> c_type = CT_RESULT;
			resposta->content.result = ret;
			break;

		case OC_GET:
			//Verifica coerencia entre codigo da operação e conteudo da mensagem
			if(message->c_type != CT_KEY) {
				return errorStruct(resposta);
			}

			int cmp;
			//verifica se cliente quer table_get ou table_get_keys
			if((cmp = strcmp(message->content.key, "!")) == 0 ) {
				char** nkeys = ptable_get_keys(tables->persistent);
				if(nkeys == NULL)
					return errorStruct(resposta);

				resposta->opcode = OC_GET + 1;
				resposta->c_type = CT_KEYS;
				resposta->content.keys = nkeys;
			}
			else {
				struct data_t *data = ptable_get(tables->persistent,message->content.key);
				
				resposta->opcode = OC_GET + 1;
				resposta->c_type = CT_VALUE;
				resposta->content.data = data;
				}

			break;
			
		case OC_DEL:
			//Verifica coerencia entre codigo da operação e conteudo da mensagem
			if(message->c_type != CT_KEY) {
				return errorStruct(resposta);
			}


			//executa pedido
			ret = ptable_del(tables->persistent, message->content.key);
			if(ret < 0)
				return errorStruct(resposta);

			//cria resposta
			resposta->opcode = OC_DEL + 1;
			resposta->c_type = CT_RESULT;
			resposta->content.result = ret;
			break;

		case OC_UPDATE:
			//Verifica coerencia entre codigo da operação e conteudo da mensagem
			if(message->c_type != CT_ENTRY) {
				return errorStruct(resposta);
			}

			//executa pedido
			ret = pable_update(tables->persistent, message->content.entry->key, message->content.entry->value);
			if(ret < 0)
				return errorStruct(resposta);

			//cria resposta
			resposta->opcode = OC_UPDATE + 1;
			resposta-> c_type = CT_RESULT;
			resposta->content.result = ret;
			break;

		case OC_SIZE:
			//executa pedido
			ret = ptable_size(tables->persistent);
			if(ret < 0)
				return errorStruct(resposta);

			//cria resposta
			resposta->opcode = OC_SIZE + 1;
			resposta-> c_type = CT_RESULT;
			resposta->content.result = ret;
			break;

		case OC_NUM_OPS:
			//executa pedido
			ret = ptable_num_ops(tables->persistent);
			if(ret < 0)
				return errorStruct(resposta);

			//cria resposta
			resposta->opcode = OC_NUM_OPS + 1;
			resposta-> c_type = CT_RESULT;
			resposta->content.result = ret;
			break;

		case OC_RT_GETTS:
		
			if(message->c_type != CT_KEY) {
				return errorStruct(resposta);
			}
			//executa pedido
			long long ret2 = ptable_get_ts(tables->persistent,message->content.key);
			if(ret2 < 0)
				return errorStruct(resposta);

			//cria resposta
			resposta->opcode = OC_RT_GETTS + 1;
			resposta-> c_type = CT_TIMESTAMP;
			resposta->content.timestamp = ret2;
			break;
		
		default:
			return errorStruct(resposta);
	}

	return resposta;
}

/*Preenche uma estrutura message_t de erro a enviar ao cliente ou NULL em caso de erro*/
struct message_t *errorStruct(struct message_t *erro) {

	//preenche a estrutura com os codigos de erro
	erro->opcode = OC_RT_ERROR;
	erro-> c_type = CT_RESULT;
	erro->content.result = -1;

	return erro;
}

