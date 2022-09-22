#ifndef EPOLL_SERVER_H_
#define EPOLL_SERVER_H_

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "clients.h"
#include "rooms.h"

#define MAX_EVENTS 64

#define DEFAULT_BUFFER_SIZE 2048

int set_non_blocking(int sockfd);
int create_and_bind(struct addrinfo *addrinfo);
int prepare_socket(const char *ip, const char *port);
void accept_client(int epoll_instance, int server_socket,
                   struct clients *list_clients);
int communicate(int epoll_instance, int client_socket,
                struct clients *list_clients, struct rooms *list_room);

#endif /* EPOLL_SERVER_H_ */
