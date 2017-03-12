/* PROJECTO 2 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */
#include "list.h"
#include "message.h"
#include "message-private.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>


/*converte um numero em 64 bits em formato de rede e vice-versa
 *Fornecido pelo Professor
*/
 long long swap_bytes_64(long long number) {

 	long long new_number;

  new_number = ((number & 0x00000000000000FF) << 56 |
                (number & 0x000000000000FF00) << 40 |
                (number & 0x0000000000FF0000) << 24 |
                (number & 0x00000000FF000000) << 8  |
                (number & 0x000000FF00000000) >> 8  |
                (number & 0x0000FF0000000000) >> 24 |
                (number & 0x00FF000000000000) >> 40 |
                (number & 0xFF00000000000000) >> 56);

  return new_number;
 }

/*preenche o buffer com o OP code e content Code comum a todas as operaçoes pretendidas
*/
void putOPandCTCode( char **msg_buf, short opCode, short ctCode) {
	short convert16;

	convert16 = htons(opCode);
	memcpy(*msg_buf, &convert16, MYSHORT);

	convert16 = htons(ctCode);
	memcpy(*msg_buf + MYSHORT, &convert16, MYSHORT);
}

/*Valida os OP codes, isto é se são um dos codigos válidos
 *retorna  O em caso afirmativo e -1 caso não seja um codigo valido
*/
 int verificaOP (short opCode) {
 	if(opCode == OC_SIZE || opCode == OC_DEL|| opCode == OC_UPDATE || opCode == OC_GET|| opCode == OC_PUT)
 		return 0;
 	else
 		return -1;
}

/*Valida o campo result da união, isto é se um valor válido
 *retorna  O em caso afirmativo e -1 caso não seja um resultado valido
*/
 int verificaTimeStamp(long timestamp){
 	return timestamp > 0 ? 0 : -1;
 }

/*Valida o campo result da união, isto é se um valor válido
 *retorna  O em caso afirmativo e -1 caso não seja um resultado valido
*/
 int verificaResult(int result){
 	return result >= -1 ? 0 : -1;
 }

 /*Valida o campo data da união, isto é se é um valor válido
 *retorna  O em caso afirmativo e -1 caso não seja um data valido
*/
 int verificaValue(struct data_t *data) {
 	return data == NULL ? -1 : 0;
 }

 /*Valida o campo Keys da união, isto é se é um valor válido
 *retorna  O em caso afirmativo e -1 caso seja null
*/
 int verificaKeys( char **keys) {
 	return keys == NULL ? -1 : 0;
 }

 /*Valida o campo Key da união, isto é se é um valor válido
 *retorna  O em caso afirmativo e -1 caso seja null
*/
 int verificaKey(char *key) {
 	return key == NULL ? -1: 0;
 }

 /*Valida o campo entry da união, isto é se é um valor válido
 *retorna  O em caso afirmativo e -1 caso seja null
*/
 int verificaEntry(struct entry_t *entry) {
 	return entry == NULL ? -1 : 0;
 }

/* Converte o conteúdo de uma message_t num char *, retornando o tamanho do
 * buffer alocado para a mensagem serializada como um array de bytes, ou -1
 * em caso de erro.
 */
int message_to_buffer(struct message_t *msg, char **msg_buf) {
	if(msg == NULL )
		return -1;

	int bufferSize;
	char *aux;
	uint32_t convert32;
	uint16_t convert16;
	long long convert64;

	switch(msg->c_type) {

		case CT_TIMESTAMP:
			//verifica parametros
			if(verificaTimeStamp(msg->content.timestamp) == -1)
				return -1;

			//aloca memoria necessaria
			bufferSize = MYSHORT + MYSHORT + MYLONG;
			*msg_buf = (char*) malloc(bufferSize);
			if(*msg_buf == NULL)
				return -1;
			//preenche buffer com o opCode + CT
			putOPandCTCode(msg_buf, msg->opcode, msg->c_type);

			//preenche o buffer com o valor de timestamp
			convert64 = swap_bytes_64(msg->content.timestamp);
			memcpy(*msg_buf + MYINT, &convert64, MYLONG);
			break;

		case CT_RESULT:
			//verifica parametros
			if(verificaResult(msg->content.result) == -1)
				return -1;

			//aloca memoria necessaria
			bufferSize = MYSHORT + MYSHORT + MYINT;
			*msg_buf = (char*) malloc(bufferSize);
			if(*msg_buf == NULL)
				return -1;
			//preenche buffer com o opCode + CT
			putOPandCTCode(msg_buf, msg->opcode, msg->c_type);

			//preenche o buffer com o valor de result 
			convert32 = htonl(msg->content.result);
			memcpy(*msg_buf + MYINT, &convert32, MYINT);
			break;

		case CT_VALUE:
			//verifica parametros
			if(verificaValue(msg->content.data) == -1)
				return -1;

			//aloca memoria necessaria
			bufferSize = MYSHORT + MYSHORT + MYINT + MYLONG + msg->content.data->datasize;
			*msg_buf = (char*) malloc(bufferSize);
			if(*msg_buf == NULL)
				return -1;
			//preenche buffer com o opCode + CT
			putOPandCTCode(msg_buf, msg->opcode, msg->c_type);

			//prenche buffer com tamanho do data e com o data
			convert32 = htonl(msg->content.data->datasize);
			memcpy(*msg_buf + MYINT, &convert32, MYINT);
			//preenche buffer com TimeStamp
			convert64 = swap_bytes_64(msg->content.data->timestamp);
			memcpy(*msg_buf + MYLONG, &convert64, MYLONG);
			//preenche buffer com conteudo data
			memcpy(*msg_buf + 16, msg->content.data->data, msg->content.data->datasize);
			break;

		case CT_KEYS:
			//verifica parametros
			if(verificaKeys(msg->content.keys) == -1)
				return -1;

			//conta numero de Keys e a memoria total que ocupam
			int i = 0;
			int strMemTotal = 0;
			while(msg->content.keys[i] != NULL) {
				strMemTotal += strlen(msg->content.keys[i]);
				i++;
			}

			//aloca memoria necessaria
			bufferSize = MYSHORT + MYSHORT + MYINT + (MYSHORT * i) + strMemTotal;
			*msg_buf = (char*) malloc(bufferSize);
			if(*msg_buf == NULL)
				return -1;
			//preenche buffer com o opCode + CT
			putOPandCTCode(msg_buf, msg->opcode, msg->c_type);

			//preenche buffer com o numero de keys
			convert32 = htonl(i);
			memcpy(*msg_buf + MYINT, &convert32, MYINT);

			//prenche buffer com tamanho e keys
			aux =  (*msg_buf) + 8;
			int index = 0;
			while(msg->content.keys[index] != NULL) {
				short length = (short) strlen(msg->content.keys[index]);
				convert16 = htons(length);
				memcpy(aux, &convert16, MYSHORT);
				aux = aux + MYSHORT;
				memcpy(aux, msg->content.keys[index], length);
				aux = aux + length;
				index++;
			}
			break;

		case CT_KEY:
			//verifica parametros
			if(verificaKey(msg->content.key) == -1)
				return -1;

			//aloca memoria necessaria
			short length = (short) strlen(msg->content.key);
			bufferSize = MYSHORT + MYSHORT + MYSHORT + length;
			*msg_buf = (char*) malloc(bufferSize);
			if(*msg_buf == NULL)
				return -1;
			//preenche buffer com o opCode + CT
			putOPandCTCode(msg_buf, msg->opcode, msg->c_type);

			//preenche buffer com o tamanho da key
			convert16 = htons(length);
			memcpy(*msg_buf + MYINT, &convert16, MYSHORT);

			//preenche buffer com a key
			memcpy(*msg_buf + 6, msg->content.key , length);
			break;

		case CT_ENTRY:
			//verifica parametros
			if(verificaEntry(msg->content.entry) == -1)
				return -1;

			//aloca memoria necessaria
			short len = (short) strlen(msg->content.entry->key);
			int dataSize = msg->content.entry->value->datasize;
			bufferSize = MYSHORT + MYSHORT + MYSHORT+ len + MYINT + MYLONG + dataSize;
			*msg_buf = (char*) malloc(bufferSize);
			if(*msg_buf == NULL)
				return -1;
			//preenche buffer com o opCode + CT
			putOPandCTCode(msg_buf, msg->opcode, msg->c_type);

			//preenche buffer com o tamanho da key
			convert16 = htons(len);
			memcpy(*msg_buf + MYINT, &convert16, MYSHORT);
			aux = (*msg_buf) + 6;

			//preenche buffer com a key
			memcpy(aux, msg->content.entry->key , len);
			aux = aux + len;

			//preenche buffer com o datasize
			convert32 = htonl(dataSize);
			memcpy(aux, &convert32, MYINT);
			aux = aux + MYINT;

			//preenche buffer com TimeStamp
			convert64 = swap_bytes_64(msg->content.entry->value->timestamp);
			memcpy(aux, &convert64, MYLONG);
			aux = aux + MYLONG;

			//preenche buffer com o data
			memcpy(aux, msg->content.entry->value->data, dataSize);
			break;

		default:
			return -1;

	}

	return bufferSize;
}

/* Transforma uma mensagem no array de bytes, buffer, para
 * uma struct message_t*
 */
struct message_t *buffer_to_message(char *msg_buf, int msg_size) {
	//verifica parametros de entrada
	if(msg_buf == NULL || msg_size <= 0)
		return NULL;

	//Aloca memoria para a estrutura message_t
	struct message_t *newMsg = (struct message_t *) malloc (sizeof(struct message_t));
	if(newMsg == NULL)
		return NULL;

	//preenche estrutura com opcode e ct_type
	short convert16;
	int convert32;
	long long convert64;
	memcpy(&convert16, msg_buf, MYSHORT);
	newMsg->opcode = ntohs(convert16);
	memcpy(&convert16, msg_buf + MYSHORT, MYSHORT);
	newMsg->c_type = ntohs(convert16);
	char * aux = msg_buf + MYINT;

	//prenche a union da estrutura dependendo do c_type;
	switch(newMsg->c_type) {

		case CT_TIMESTAMP:
			//preenche campo timestamp de content_u
			memcpy(&convert64, aux, MYLONG);
			newMsg->content.timestamp = swap_bytes_64(convert64);
			break;

		case CT_RESULT:
			//preenche campo Result de content_u
			memcpy(&convert32, aux, MYINT);
			newMsg->content.result = ntohl(convert32);
			break;

		case CT_VALUE:
			//preenche campo data de content_u
			memcpy(&convert32,aux, MYINT);
			aux = aux + MYINT;
			memcpy(&convert64, aux, MYLONG);
			aux = aux + MYLONG;
			newMsg->content.data = data_create(ntohl(convert32));
			if(newMsg->content.data == NULL) {
				free(newMsg);
				return NULL;
			}
			//preenche campo data->timestamp de message
			newMsg->content.data->timestamp = swap_bytes_64(convert64);
			memcpy(newMsg->content.data->data, aux, ntohl(convert32));
			
			break;

		case CT_KEY:
			//preenche campo key de content_u
			memcpy(&convert16, aux, MYSHORT);
			short length = ntohs(convert16);
			newMsg->content.key = (char*) malloc(length+1);
			memcpy(newMsg->content.key, aux + MYSHORT, length);
			newMsg->content.key[length] = '\0';
			break;

		case CT_KEYS:
			//preenche campo keys de content_u
			memcpy(&convert32, aux, MYINT);
			int nKeys = ntohl(convert32);
			//aloca memoria para o vector de Keys
			newMsg->content.keys = (char**) malloc(sizeof(char*)*(nKeys + 1));
			
			if(newMsg->content.keys == NULL) {
				free(newMsg);
				return NULL;
			}
			aux = aux + MYINT;
			int i;
		
			//preenche cada posição do vector de keys com a key
			for(i = 0; i < nKeys; i++) {
				memcpy(&convert16, aux, MYSHORT);
				aux = aux + MYSHORT;
				short len = ntohs(convert16);
				
				//aloca memoria para cada key
				newMsg->content.keys[i] = (char*) malloc((len+1));
				if(newMsg->content.keys[i] == NULL) {
					int j;
					for( j = 0; j < i; j++) {
						free(newMsg->content.keys[j]);
					}
					free(newMsg->content.keys);
					free(newMsg);
					return NULL;
				}
				memcpy(newMsg->content.keys[i], aux, len);
				newMsg->content.keys[i][len] = '\0';
				
				aux = aux + len;
			}
			newMsg->content.keys[nKeys] = NULL;
			break;

		case CT_ENTRY:
			//determina keySize e aloca memoria para receber a key
			memcpy(&convert16, aux, MYSHORT);
			short len2 = ntohs(convert16);
			char * entryKey = (char*) malloc(len2 +1);
			if(entryKey == NULL) {
				free(newMsg);
				return NULL;
			}
			//recebe a Key
			aux = aux + MYSHORT;
			memcpy(entryKey, aux, len2);
			entryKey[len2] = '\0';
			//determina datasize e cria um data_t com esse valor
			aux = aux + len2;
			int dataSize;
			memcpy(&dataSize, aux, MYINT);
			aux = aux + MYINT;
			memcpy(&convert64, aux, MYLONG);
			struct data_t *data = data_create(ntohl(dataSize));
			if(data == NULL) {
				free(entryKey);
				free(newMsg);
				return NULL;
			}
			
			data->timestamp = swap_bytes_64(convert64);
			//recebe os dados
			aux = aux + MYLONG;
			memcpy(data->data, aux, ntohl(dataSize));

			//preenche o campo entry da union_t e liberta memoria reservada
			newMsg->content.entry = entry_create(entryKey, data);
			data_destroy(data);
			free(entryKey);
			if(newMsg->content.entry == NULL) {
				free(newMsg);
				return NULL;
			}
			break;

		default:
			free(newMsg);
			return NULL;
	}

	return newMsg;

}

/* Liberta a memoria alocada na função buffer_to_message
 */
void free_message(struct message_t *msg) {

	if(msg != NULL) {

		switch (msg->c_type) {

			case CT_VALUE:
				data_destroy(msg->content.data);
				free(msg);
				break;

			case CT_KEY:
				free(msg->content.key);
				free(msg);
				break;

			case CT_KEYS:
				list_free_keys(msg->content.keys);
				free(msg);
				break;

			case CT_ENTRY:
				entry_destroy(msg->content.entry);
				free(msg);
				break;

			default:
				free(msg);
		}

	}
}

