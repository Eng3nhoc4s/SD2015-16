#!/bin/sh
## PROJECTO 4 - Sistemas Distribuidos - 2015/2016
## 40251	José Albuquerque
## 40853	António Rodrigues
## 41941	Rodrigo Reis

##PATHS
ROOT = .
INC = $(ROOT)/inc
OBJ = $(ROOT)/obj
SRC = $(ROOT)/src
TBL = $(SRC)/table
NTW = $(SRC)/network
CLT = $(SRC)/client
SRV = $(SRC)/server
PRS = $(SRC)/persistence
QRM = $(SRC)/quorum

##FLAGS
CC = gcc
FLAGS = -I$(INC) -lm -Wall
FLAGS2 = -pthread -I$(INC) -lm -Wall -pthread

##SERVER OBJECTS
SERVER_OBJECTS = $(OBJ)/data.o $(OBJ)/entry.o $(OBJ)/table.o $(OBJ)/list.o $(OBJ)/message.o $(OBJ)/table_server.o $(OBJ)/communication.o $(OBJ)/persistence_manager.o $(OBJ)/persistent_table.o $(OBJ)/table_skel.o 

##CLIENT OBJECTS
CLIENT_OBJECTS = $(OBJ)/data.o $(OBJ)/entry.o $(OBJ)/table.o $(OBJ)/list.o $(OBJ)/message.o $(OBJ)/network_client.o $(OBJ)/table_client.o $(OBJ)/communication.o $(OBJ)/client_stub.o $(OBJ)/quorum_table.o $(OBJ)/quorum_access.o $(OBJ)/queue.o 



##TARGETS
all: clean server client

server: $(SERVER_OBJECTS)
	$(CC) $(SERVER_OBJECTS) -o server
client: $(CLIENT_OBJECTS)
	$(CC) -pthread $(CLIENT_OBJECTS) -o client -lm


##OBJECTOS
##QUINTO PROJECTO
$(OBJ)/quorum_table.o: $(QRM)/quorum_table.c $(INC)/quorum_table.h $(INC)/quorum_table-private.h
	$(CC) $(FLAGS) -c $(QRM)/quorum_table.c -o $(OBJ)/quorum_table.o

$(OBJ)/quorum_access.o: $(QRM)/quorum_access.c $(INC)/quorum_access.h $(INC)/quorum_access-private.h $(INC)/queue.h
	$(CC) $(FLAGS2) -c $(QRM)/quorum_access.c -o $(OBJ)/quorum_access.o

$(OBJ)/queue.o: $(QRM)/queue.c $(INC)/queue.h
	$(CC) $(FLAGS) -c $(QRM)/queue.c -o $(OBJ)/queue.o

##QUARTO PROJECTO
$(OBJ)/client_stub.o: $(CLT)/client_stub.c $(INC)/client_stub-private.h $(INC)/data.h $(INC)/table.h 
	$(CC) $(FLAGS) -c $(CLT)/client_stub.c -o $(OBJ)/client_stub.o
	
$(OBJ)/persistence_manager.o: $(PRS)/persistence_manager.c $(INC)/table-private.h $(INC)/table.h
	$(CC) $(FLAGS) -c $(PRS)/persistence_manager.c -o $(OBJ)/persistence_manager.o
	
$(OBJ)/persistent_table.o: $(PRS)/persistent_table.c $(INC)/data.h $(INC)/table-private.h $(INC)/persistence_manager-private.h $(INC)/persistent_table-private.h
	$(CC) $(FLAGS) -c $(PRS)/persistent_table.c -o $(OBJ)/persistent_table.o
	
$(OBJ)/table_skel.o: $(SRV)/table_skel.c $(INC)/message.h
	$(CC) $(FLAGS) -c $(SRV)/table_skel.c -o $(OBJ)/table_skel.o

##TERCEIRO PROJECTO
$(OBJ)/communication.o: $(NTW)/communication.c $(INC)/communication.h $(INC)/message.h $(INC)/message-private.h
	$(CC) $(FLAGS) -c $(NTW)/communication.c -o $(OBJ)/communication.o

$(OBJ)/network_server.o: $(SRV)/network_server.c $(INC)/network_server.h $(INC)/table.h $(INC)/data.h $(INC)/message.h $(INC)/message-private.h
	$(CC) $(FLAGS) -c $(SRV)/network_server.c -o $(OBJ)/network_server.o

$(OBJ)/table_server.o: $(SRV)/table_server.c $(INC)/table.h $(INC)/message.h $(INC)/message-private.h
	$(CC) $(FLAGS) -c $(SRV)/table_server.c -o $(OBJ)/table_server.o

$(OBJ)/network_client.o: $(CLT)/network_client.c $(INC)/network_client.h $(INC)/network_client-private.h $(INC)/network_server.h
	$(CC) $(FLAGS) -c $(CLT)/network_client.c -o $(OBJ)/network_client.o

$(OBJ)/table_client.o: $(CLT)/table_client.c $(INC)/data.h $(INC)/entry.h $(INC)/message.h $(INC)/message-private.h $(INC)/network_client.h $(INC)/network_client.h $(INC)/quorum_table.h 
	$(CC) $(FLAGS) -c $(CLT)/table_client.c -o $(OBJ)/table_client.o

##SEGUNDO PROJECTO
$(OBJ)/table.o: $(TBL)/table.c $(INC)/table-private.h $(INC)/list.h $(INC)/list-private.h
	$(CC) $(FLAGS) -c $(TBL)/table.c -o $(OBJ)/table.o

$(OBJ)/message.o: $(NTW)/message.c $(INC)/message.h $(INC)/data.h $(INC)/entry.h $(INC)/message-private.h
	$(CC) $(FLAGS) -c $(NTW)/message.c -o $(OBJ)/message.o
	
##PRIMEIRO PROJECTO
$(OBJ)/data.o: $(TBL)/data.c $(INC)/data.h
	$(CC) $(FLAGS) -c $(TBL)/data.c -o $(OBJ)/data.o

$(OBJ)/entry.o: $(TBL)/entry.c $(INC)/data.h $(INC)/entry.h
	$(CC) $(FLAGS) -c $(TBL)/entry.c -o $(OBJ)/entry.o

$(OBJ)/list.o: $(TBL)/list.c $(INC)/list.h $(INC)/list-private.h
	$(CC) $(FLAGS) -c $(TBL)/list.c -o $(OBJ)/list.o

##TESTES
$(OBJ)/test_table.o: $(SRC)/test_table.c $(INC)/table.h
	$(CC) $(FLAGS) -c $(SRC)/test_table.c -o $(OBJ)/test_table.o

$(OBJ)/test_message.o: $(SRC)/test_message.c $(INC)/message.h
	$(CC) $(FLAGS) -c $(SRC)/test_message.c -o $(OBJ)/test_message.o

##DUMMY TARGETS
backup:
	tar -czvf ./`date +%Y.%m.%d-%H:%M:%S`.tar * --exclude='*~' --exclude='*.tar'

clean:
	rm -fr obj/*.o *~ server client



