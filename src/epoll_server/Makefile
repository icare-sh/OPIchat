CC = gcc
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -fsanitize=address
BIN = epoll_server
OBJ = utils/xalloc.c connection.c epoll-server.c

epoll_server:
	$(CC) $(CFLAGS) -o $(BIN) $(OBJ)
	
clean: 
	$(RM) $(BIN)
