/* PROJECTO 2 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

#ifndef _MSGPRIVATE_H
#define _MSGPRIVATE_H

/*define novo opcode de erro*/
#define OC_RT_ERROR	99
#define CT_TIMESTAMP 60
 
#define MYSHORT 2
#define MYINT 4
#define MYLONG 8

#include "message.h"

/*preenche o buffer com o OP code e content Code comum a todas as operaçoes pretendidas
*/
void putOPandCTCode( char **msg_buf, short opCode, short ctCode);

/*Valida os OP codes, isto é se são um dos codigos válidos
 *retorna  O em caso afirmativo e -1 caso não seja um codigo valido
*/
 int verificaOP (short opCode);

/*Valida o campo entry da união, isto é se é um valor válido
 *retorna  O em caso afirmativo e -1 caso seja null
*/
 int verificaEntry(struct entry_t *entry);

/*Valida o campo Key da união, isto é se é um valor válido
 *retorna  O em caso afirmativo e -1 caso seja null
*/
 int verificaKey(char *key);

/*Valida o campo Keys da união, isto é se é um valor válido
 *retorna  O em caso afirmativo e -1 caso seja null
*/
 int verificaKeys( char **keys);

/*Valida o campo data da união, isto é se é um valor válido
 *retorna  O em caso afirmativo e -1 caso não seja um data valido
*/
 int verificaValue(struct data_t *data);

/*Valida o campo Result da união, isto é se  é um valor válido
 *retorna  O em caso afirmativo e -1 caso não seja um resultado valido
*/
 int verificaResult(int result);

/*converte um numero em 64 bits em formato de rede e vice-versa
*/
 long long swap_bytes_64(long long number);

#endif