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
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "connection.h"

int set_non_blocking(int sockfd);

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
        printf("Server do not listen\n");
    else
        fprintf(stdout, "Listenning ....\n");

    freeaddrinfo(result);
    return res;
}

struct connection_t *accept_client(int epoll_instance, int server_socket,
                                   struct connection_t *connection)
{
    int client_socket = accept(server_socket, NULL, NULL);

    if (client_socket == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    struct connection_t *con;
    struct epoll_event ev;

    if (epoll_instance == -1)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    set_non_blocking(client_socket);
    ev.events = EPOLLIN;
    ev.data.fd = client_socket;
    if (epoll_ctl(epoll_instance, EPOLL_CTL_ADD, client_socket, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    con = add_client(connection, client_socket);
    return con;
}

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

struct connection_t *communicate(struct connection_t *connection,
                                 int client_socket, int epoll_instance)
{
    struct connection_t *connect;
    connect = find_client(connection, client_socket);

    struct connection_t *tmp = connection;
    char buf[2];
    int rec = 0;
    connect->buffer = NULL;
    connect->buffer = malloc(sizeof(char));
    connect->nb_read = 0;
    do
    {
        connect->buffer = realloc(connect->buffer, connect->nb_read + 2);
        rec = recv(client_socket, buf, 1, 0);
        if (rec == -1)
            break;
        connect->buffer[connect->nb_read] = buf[0];
        connect->nb_read += 1;
    } while (rec == 1 && connect->buffer[connect->nb_read - 1] != '\n');
    printf("connect->nb_read : %ld\n", connect->nb_read);
    connect->buffer[connect->nb_read] = '\0';
    printf("ICI : connect->nb_read : %ld\n", connect->nb_read);
    if (strchr(connect->buffer, '\n') == NULL)
        send(client_socket, "", 0, MSG_NOSIGNAL);
    if (rec == 0)
    {
        epoll_ctl(epoll_instance, EPOLL_CTL_DEL, client_socket, NULL);
        fprintf(stdout, "Client [%d] Disconnected \n", client_socket);
        connection = remove_client(connection, client_socket);
    }
    else
    {
        fprintf(stdout, "Received by client : %s", connect->buffer);
        while (tmp != NULL)
        {
            send(tmp->client_socket, connect->buffer, connect->nb_read,
                 MSG_NOSIGNAL);
            tmp = tmp->next;
        }
    }
    // free(connect->buffer);
    return connection;
}

int main(int argc, char *argv[])
{
    struct epoll_event event, events[MAX_EVENTS];
    int epollfd = epoll_create1(0);
    struct connection_t *connect;
    if (epollfd == -1)
    {
        fprintf(stderr, "Failed to create epoll file descriptor\n");
        return 1;
    }

    int sockfd;
    if (argc == 3)
        sockfd = prepare_socket(argv[1], argv[2]);

    event.events = EPOLLIN;
    event.data.fd = sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) == -1)
    {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == sockfd)
            {
                connect = accept_client(epollfd, sockfd, connect);
                fprintf(stdout, "client Connected: [%d]\n", events[n].data.fd);
            }
            else if (events[n].events & EPOLLIN)
                connect = communicate(connect, events[n].data.fd, epollfd);
            else
            {
                connect = remove_client(connect, events[n].data.fd);
                fprintf(stdout, "Client [%d] Disconnected \n",
                        events[n].data.fd);
            }
        }
    }
    close(sockfd);
    close(epollfd);
    return 0;
}
