#ifndef _TABLE_SKEL_PRIVATE_H
#define _TABLE_SKEL_PRIVATE_H

#include "message.h"
#include "table.h"
#include "persistent_table.h"

struct table_skel_t {
	struct ptable_t *persistent;
	struct pmanager_t *pmanager;
};

/* Inicia o skeleton da tabela.
 * O main() do servidor deve chamar esta função antes de poder usar a
 * função invoke(). O parâmetro n_lists define o número de listas a
 * serem usadas pela tabela mantida no servidor.
 * Recebe tambem uma string com o nome do ficheiro de Log.
 * Retorna 0 (OK) ou -1 (erro, por exemplo OUT OF MEMORY)
 */
int table_skel_init(int n_lists, char * filename);

/*Preenche uma estrutura message_t de erro a enviar ao cliente ou NULL em caso de erro*/
struct message_t *errorStruct(struct message_t *erro);


#endif
