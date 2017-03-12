#ifndef _PERSISTENCE_MANAGER_PRIVATE_H
#define _PERSISTENCE_MANAGER_PRIVATE_H
#include <stdio.h>
#include <unistd.h>

#define LOG_SIZE 50

struct pmanager_t {
	FILE *file;
	int logsize;
	char * filename;
	FILE * fileSTT;
	FILE * fileCHK;
};

/* Cria um ficheiro filename+".stt" com o estado atual da tabela table.
* Retorna o tamanho do ficheiro criado ou -1 em caso de erro.
*/
int pmanager_store_table(struct pmanager_t *pmanager,
struct table_t *table);
/* Limpa o conteúdo do ficheiro ".log" e copia o ficheiro ".stt" para ".ckp".
* Retorna 0 se tudo correr bem ou -1 em caso de erro.
*/
int pmanager_rotate_log(struct pmanager_t *pmanager);

char *processFilename(char * str, char * ext);

int sizeFile(FILE *file);

/*
 * RESTAURA A TABELA ATRAVES DO FICHEIRO CUJA EXTENSAO FOR FORNECIDA
 */
int pmanager_restore_table(struct pmanager_t *pmanager, struct table_t *table, char * extension);

/*
* COPIA UM FICHEIRO NA SUA INTEGRA
*/

int copy_file(char *stt, char  *chk);

#endif
