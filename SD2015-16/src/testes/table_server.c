/* PROJECTO 3 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#include "message.h"
#include "message-private.h"
#include "table.h"
#include "network_server.h"
#include "data.h"
#include <string.h>
#include <stdlib.h>

/*Preenche uma estrutura message_t de erro a enviar ao cliente ou NULL em caso de erro*/
struct message_t *errorStruct(struct message_t *erro) {

	//preenche a estrutura com os codigos de erro
	erro->opcode = OC_RT_ERROR;
	erro-> c_type = CT_RESULT;
	erro->content.result = -1;

	return erro;
}


/* Executa o pedido enviado pelo cliente e retorna um estrutura message_t com a resposta*/
struct message_t *executa_e_responde(struct table_t *table, struct message_t *message) {
	
	//XXXXXXXXXXXXXXXXXXXXXXX PARA APAGAR:
	printf("E_e_R: 1-message->content.entry->key= %s\n", message->content.entry->key );


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
			ret = table_put(table, message->content.entry->key, message->content.entry->value);
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
				char** nkeys = table_get_keys(table);
				if(nkeys == NULL)
					return errorStruct(resposta);

				resposta->opcode = OC_GET + 1;
				resposta->c_type = CT_KEYS;
				resposta->content.keys = nkeys;
			}
			else {
				struct data_t *data = table_get(table,message->content.key);
				if(data == NULL) {
					resposta->opcode = OC_GET + 1;
					resposta->c_type = CT_VALUE;
					resposta->content.data = data_create(0);
				}
				else {
					resposta->opcode = OC_GET + 1;
					resposta->c_type = CT_VALUE;
					resposta->content.data = data;
				}

			}
			break;
			
		case OC_DEL:
			//Verifica coerencia entre codigo da operação e conteudo da mensagem
			if(message->c_type != CT_KEY) {
				return errorStruct(resposta);
			}

			//executa pedido
			ret = table_del(table, message->content.key);
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
			ret = table_update(table, message->content.entry->key, message->content.entry->value);
			if(ret < 0)
				return errorStruct(resposta);

			//cria resposta
			resposta->opcode = OC_UPDATE + 1;
			resposta-> c_type = CT_RESULT;
			resposta->content.result = ret;
			break;

		case OC_SIZE:
			//Verifica coerencia entre codigo da operação e conteudo da mensagem
			if(message->c_type != CT_ENTRY) {
				return errorStruct(resposta);
			}

			//executa pedido
			ret = table_size(table);
			if(ret < 0)
				return errorStruct(resposta);

			//cria resposta
			resposta->opcode = OC_SIZE + 1;
			resposta-> c_type = CT_RESULT;
			resposta->content.result = ret;
			break;
		
		default:
			return errorStruct(resposta);
	}

	return resposta;
}




int main(int argc, char **argv){


	// Verifica se foi passado algum argumento
	if (argc != 3) {
    	printf("Uso: ./<nome_executavel> <porto_servidor> <num_listas>\n");
    	printf("Exemplo de uso: ./server 12345 12\n");
 		return -1;
 	}

 	
 	//cria tabela
 	struct table_t *table = table_create( atoi(argv[2]) );
 	if(table == NULL) {
 		perror("erro a criar tabela");
 		return -1;
 	}

 	//cria socket faz bind e listen
 	int socket_id = create_connection( atoi(argv[1]) );
 	if(socket_id < 0) {
 		table_destroy(table);
 		perror("erro a criar coneccao");
 		return -1;
 	}
 	
 	printf("Servidor à espera de dados\n");


 	
 	struct sockaddr_in client;
 	socklen_t size_client;

 	int connect_socket_id = accept(socket_id,(struct sockaddr *) &client, &size_client);
 	if(connect_socket_id < 0) {
 		perror("Erro na aceitacao");
 		close(socket_id);
 		return -1 ;
 	}

 	char * bufferLido;
 	int bufferSize;

 	//enquanto cliente enviar dados
 	while((bufferSize = read_all(connect_socket_id, &bufferLido)) > 0) {

 		//Transforma dados recebidos pela rede(buffer) numa estrutura message
 		struct message_t *pedido = buffer_to_message(bufferLido, bufferSize);
		if(pedido == NULL) {
			free(bufferLido);
			continue;
		}

		//Executa pedido e prepara resposta
		struct message_t *resposta = executa_e_responde(table,pedido);
		if(resposta == NULL) {
			free_message(pedido);
			free(bufferLido);
			continue;
		}

		//cria buffer de resposta
		char * msg_buf;
		int tamanho = message_to_buffer(resposta, &msg_buf);
		if(tamanho < 0) {
			free_message(pedido);
			free_message(resposta);
			free(bufferLido);
			continue;
		}

		// Envia resposta ao cliente pelo socket referente a conexão
		tamanho= write_all(connect_socket_id, msg_buf, tamanho);
		if(tamanho == -1) {
			free_message(pedido);
			free_message(resposta);
			free(bufferLido);
			free(msg_buf);
			continue;
		}

		free(bufferLido);

 	}
 /*
 	// Bloqueia a espera de pedidos de conexão
    while ((connect_socket_id = accept(socket_id,(struct sockaddr *) &client, &size_client)) != -1) {

		// Lê buffer enviado pelo cliente do socket referente a conexão
		char * bufferLido;
		int bufferSize = read_all(connect_socket_id, &bufferLido);
		if(bufferSize == -1) {
			close(connect_socket_id);
			continue;
		}

		struct message_t *pedido = buffer_to_message(bufferLido, bufferSize);
		if(pedido == NULL) {
			close(connect_socket_id);
			free(bufferLido);
			continue;
		}

		//Executa pedido e prepara resposta
		struct message_t *resposta = executa_e_responde(table,pedido);
		if(resposta == NULL) {
			close(connect_socket_id);
			free_message(pedido);
			free(bufferLido);
			continue;
		}

		//cria buffer de resposta
		char * msg_buf;
		bufferSize = message_to_buffer(resposta, &msg_buf);

		// Envia resposta ao cliente pelo socket referente a conexão
		bufferSize = write_all(connect_socket_id, msg_buf, bufferSize);
		if(bufferSize == -1) {
			close(connect_socket_id);
			free_message(pedido);
			free_message(resposta);
			free(bufferLido);
			free(msg_buf);
			continue;
		}
		
		
		// Fecha socket referente a esta conexão e liberta memoria
		close(connect_socket_id);
		free_message(pedido);
		free_message(resposta);
		free(bufferLido);
		free(msg_buf);

	}
*/
	// Fecha sockets
	close(connect_socket_id);
	close(socket_id);
	table_destroy(table);
	
	return 0;
}