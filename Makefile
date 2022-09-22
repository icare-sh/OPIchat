CC = gcc
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -fsanitize=address
BIN_SERVER = opichat_server
OBJ_SERVER = src/opichat_server.c src/payload.c src/server/clients.c src/server/epoll-server.c src/server/check-payload.c src/server/rooms.c
BIN_CLIENT = opichat_client
OBJ_CLIENT = src/opichat_client.c src/payload.c src/client/request.c src/client/receive.c
CCRITERION = -lcriterion

all:	opichat_server opichat_client

opichat_server:
	$(CC) $(CFLAGS) -o $(BIN_SERVER) $(OBJ_SERVER) -lpthread
	
opichat_client:
	$(CC) $(CFLAGS) -o $(BIN_CLIENT) $(OBJ_CLIENT) -lpthread
check:
	$(CC) $(CFLAGS) -o $(OBJ_SERVER) $(OBJ_CLIENT) $(CCRITERION) -lpthread

clean: 
	$(RM) $(BIN_SERVER) $(BIN_CLIENT) 
