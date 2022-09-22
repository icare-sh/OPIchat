#define _POSIX_C_SOURCE 200112L

#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client/receive.h"
#include "client/request.h"

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

struct arg_thread
{
    int socket;
};

void *thread1(void *argts)
{
    struct arg_thread *arg = argts;
    while (1)
        request(arg->socket);
    pthread_exit(NULL);
}

void *thread2(void *argts)
{
    struct arg_thread *arg = argts;
    while (1)
        receive(arg->socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    int sck;
    pthread_t t1;
    pthread_t t2;
    if (argc == 3)
        sck = prepare_socket(argv[1], argv[2]);
    else
    {
        fprintf(stderr, "student: usage: /student <ip> <port>\n");
        exit(1);
    }
    struct arg_thread arg;
    arg.socket = sck;
    pthread_create(&t1, NULL, thread1, (void *)&arg);
    pthread_create(&t2, NULL, thread2, (void *)&arg);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}
