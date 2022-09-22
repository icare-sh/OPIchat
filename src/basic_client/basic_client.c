#define _POSIX_C_SOURCE 200112L

#include "basic_client.h"

#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int create_and_connect(struct addrinfo *addrinfo)
{
    int res;
    struct addrinfo *r;
    for (r = addrinfo; r; r = r->ai_next)
    {
        res = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
        if (res == -1)
            continue;
        if (connect(res, r->ai_addr, r->ai_addrlen) != -1)
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

    int res = create_and_connect(result);

    freeaddrinfo(result);
    return res;
}

void communicate(int server_socket)
{
    while (1)
    {
        char buffer[10];
        char *msg = calloc(1, sizeof(char));
        fprintf(stderr, "Enter your message:\n");
        while (fgets(buffer, 10, stdin))
        {
            msg = realloc(msg, strlen(msg) + 1 + strlen(buffer));
            strcat(msg, buffer);
            if (msg[strlen(msg) - 1] == '\n')
                break;
        }

        if (strlen(msg) == 0)
        {
            close(server_socket);
            return;
        }
        if (send(server_socket, msg, strlen(msg), 0) == -1)
            perror("ERROR SEND");

        char rep[2];
        recv(server_socket, rep, 1, 0);
        fprintf(stdout, "Server answered with: %s", rep);
        while (rep[0] != '\n')
        {
            recv(server_socket, rep, 1, 0);
            fprintf(stdout, "%s", rep);
        }
        free(msg);
    }
}

int main(int argc, char *argv[])
{
    int sck;
    if (argc == 3)
        sck = prepare_socket(argv[1], argv[2]);
    else
    {
        fprintf(stderr, "Usage: ./basic_client SERVER_IP SERVER_PORT\n");
        exit(1);
    }
    communicate(sck);
    return 0;
}
