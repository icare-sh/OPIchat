#define _POSIX_C_SOURCE 200112L
#define BACKLOG 10
#define _GNU_SOURCE
#define SIZE 4026

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
#include <threads.h>
#include <unistd.h>

#include "payload.h"
#include "server/check-payload.h"
#include "server/clients.h"
#include "server/epoll-server.h"
#include "server/rooms.h"

int main(int argc, char *argv[])
{
    struct epoll_event event, events[MAX_EVENTS];
    int epollfd = epoll_create1(0);
    struct clients *list_clients = clients_init();
    struct rooms *r = rooms_init();
    int sockfd;
    if (epollfd == -1)
        errx(1, "Failed to create epoll fd");

    if (argc == 3)
        sockfd = prepare_socket(argv[1], argv[2]);
    else
        errx(1, "usage: /student <ip> <port>");

    event.events = EPOLLIN;
    event.data.fd = sockfd;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) == -1)
        errx(1, "Failed to listen socket from epoll_ctl");

    while (1)
    {
        if (list_clients == NULL || list_clients->head == NULL)
            list_clients = clients_init();
        if (r == NULL || r->head == NULL)
            r = rooms_init();

        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1)
            errx(1, "Failed from epoll_wait");
        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == sockfd)
                accept_client(epollfd, sockfd, list_clients);
            else if (events[n].events & EPOLLIN)
            {
                if (communicate(epollfd, events[n].data.fd, list_clients, r))
                    check_payload(epollfd, events[n].data.fd, list_clients, r,
                                  argv[1]);
            }
            else
            {
                clients_remove_socket(list_clients, events[n].data.fd, r);
                fprintf(stdout, "Client [%d] Disconnected \n",
                        events[n].data.fd);
            }
        }
    }
    close(sockfd);
    close(epollfd);
    return 0;
}
