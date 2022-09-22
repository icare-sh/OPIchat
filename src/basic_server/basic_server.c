#define _POSIX_C_SOURCE 200112L
#define BACKLOG 10

#include "basic_server.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
    int ret = getaddrinfo(ip, port, &hints, &result);
    if (ret != 0)
        errx(EXIT_FAILURE, "getaddrinfo : %s", gai_strerror(ret));
    int res = create_and_bind(result);
    listen(res, BACKLOG);
    freeaddrinfo(result);
    return res;
}

int accept_client(int socket)
{
    struct sockaddr_storage client_addr;
    socklen_t addr_size;

    addr_size = sizeof client_addr;
    int new_fd = accept(socket, (struct sockaddr *)&client_addr, &addr_size);
    if (new_fd != -1)
        fprintf(stdout, "Client connected\n");
    return new_fd;
}

void communicate(int client_socket)
{
    char buf[2];
    while (1)
    {
        char *msg = calloc(1, sizeof(char));
        int i = 0;
        int rec = 0;
        do
        {
            msg = realloc(msg, i + 2);
            rec = recv(client_socket, buf, 1, 0);
            if (rec == -1)
                break;
            msg[i] = buf[0];
            i = i + 1;
        } while (rec == 1 && msg[i - 1] != '\n');
        msg[i] = '\0';
        if (strchr(msg, '\n') == NULL)
            send(client_socket, "", 0, MSG_NOSIGNAL);
        else
        {
            fprintf(stdout, "Received Body: %s", msg);
            send(client_socket, msg, i, MSG_NOSIGNAL);
        }
        if (rec == 0)
            break;
        free(msg);
    }
    close(client_socket);
}

int main(int argc, char *argv[])
{
    int sck;
    if (argc == 3)
        sck = prepare_socket(argv[1], argv[2]);
    else
    {
        fprintf(stderr, "Usage: ./basic_server SERVER_IP SERVER_PORT\n");
        exit(1);
    }
    while (1)
    {
        int sck_client = accept_client(sck);
        if (sck_client == -1)
            return 1;
        communicate(sck_client);
        fprintf(stdout, "Client disconnected\n");
    }
    close(sck);
    return 0;
}
