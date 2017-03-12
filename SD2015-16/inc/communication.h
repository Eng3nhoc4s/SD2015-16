/* PROJECTO 3 - Sistemas Distribuidos - 2015/2016
 * 40251	José Albuquerque
 * 40853	António Rodrigues
 * 41941	Rodrigo Reis
 */

/*Lê buffer de comunicação e guarda a informaçao num apontador
 * Retorna o nº de bytes lido, 0 caso a comunicacão falhe ou -1 em caso de erro
 */
int read_all( int sockfd, void * pointer, int size );

/*Escreve num buffer de comunicação a informaçao da variavel pointer
 *Retorna o numero de bytes escrito, -1 em caso de erro ou 0 caso ligaçao tenha terminado
 */
int write_all( int sockfd, void * pointer, int size );

/*envia para o cliente a resposta do servidor*/
/*Protocolo: envia primeiro o tamanho da mensagem a enviar(4 bytes) e depois envia a mensagem*/ 
/*Retorna o tamanho do buffer com a mensagem  enviado -1 em caso de erro ou 0 caso tenha terminado ligaçao */ 
int write_all2( int sockID, char *msg, int bufferSize);

/*Lê todo buffer de comunicação e guarda a informaçao em msg_buffer
 *Protocolo: lê 4 bytes com o tamanho da mensagem e de seguida lê mensagem com esse tamaho.
 *Retorna o tamanho do buffer com a mensagem,-1 em caso de erro, 
 * ou 0 caso cliente tenha fechado ligação*/ 
int read_all2( int sockfd, char ** msg_buf );
