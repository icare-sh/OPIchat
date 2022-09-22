#define _POSIX_C_SOURCE 200112L
#define BACKLOG 10
#define _GNU_SOURCE
#define SIZE 4026
#include "epoll-server.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "clients.h"

int set_non_blocking(int sockfd)
{
    int flags, s;
    flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl");
        return -1;
    }
    flags |= O_NONBLOCK;
    s = fcntl(sockfd, F_SETFL, flags);
    if (s == -1)
    {
        perror("fcntl");
        return -1;
    }
    return 0;
}

int create_and_bind(struct addrinfo *addrinfo)
{
    int res;
    struct addrinfo *r;
    for (r = addrinfo; r; r = r->ai_next)
    {
        res = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
        if (res == -1)
            continue;

        if (bind(res, r->ai_addr, r->ai_addrlen) != -1)
            break;
        close(res);
    }
    if (!r)
        return 1;
    return res;
}

int prepare_socket(const char *ip, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int ret = getaddrinfo(ip, port, &hints, &result);

    if (ret != 0)
        errx(EXIT_FAILURE, "getaddrinfo : %s", gai_strerror(ret));
    int res = create_and_bind(result);
    if (listen(res, BACKLOG) == -1)
        errx(1, "Server don't listen");
    else
        fprintf(stdout, "Listenning ...\n");

    freeaddrinfo(result);
    return res;
}

void accept_client(int epoll_instance, int server_socket,
                   struct clients *list_clients)
{
    struct epoll_event ev;
    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket == -1)
        errx(1, "Accept_client : Failed to accept client");

    if (epoll_instance == -1)
        errx(1, "Accept_client : Failed to create epoll_instance");

    set_non_blocking(client_socket);
    ev.events = EPOLLIN;
    ev.data.fd = client_socket;

    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, client_socket, &ev) == -1)
        errx(1, "Accept_client : Failed to listen_socket from epoll_ctl");

    clients_push(list_clients, client_socket);
    fprintf(stdout, "Client connected\n");
}

int communicate(int epoll_instance, int client_socket,
                struct clients *list_clients, struct rooms *list_room)
{
    struct client_item *client =
        client_get_by_socket(list_clients, client_socket);

    char buf[2];
    int rec = 0;
    char *msg = malloc(sizeof(char));
    int msg_len = 0;
    do
    {
        msg = realloc(msg, msg_len + 2);
        rec = recv(client_socket, buf, 1, 0);
        if (rec == -1)
            break;
        msg[msg_len] = buf[0];
        msg_len += 1;
    } while (rec == 1);
    msg[msg_len] = '\0';
    if (rec == 0)
    {
        epoll_ctl(epoll_instance, EPOLL_CTL_DEL, client_socket, NULL);
        clients_remove_socket(list_clients, client_socket, list_room);
        return 0;
    }
    client_set_msg(client, msg);
    free(msg);
    return 1;
}
