#include "receive.h"

#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../payload.h"

void receive(int server_socket)
{
    char rep[DEFAULT_BUFFER_SIZE];
    memset(rep, 0, DEFAULT_BUFFER_SIZE);
    recv(server_socket, rep, DEFAULT_BUFFER_SIZE, 0);
    struct payload_t *p = txt_to_payload(rep);
    if (p->success != 0)
    {
        close(server_socket);
        free_payload(p);
        exit(1);
    }
    if (p->status == 1 && strlen(p->msg) > 0)
        fprintf(stdout, "< %s", p->msg);
    if (p->status == 3 && strlen(p->msg) > 0)
        fprintf(stderr, "! %s", p->msg);
    if (p->status == 2 && strlen(p->msg) > 0)
    {
        if (strcmp(p->command, "SEND-DM") == 0)
        {
            char username[DEFAULT_BUFFER_SIZE];
            strcpy(username, p->parameters[1] + 5);
            fprintf(stdout, "From %s: %s\n", username, p->msg);
        }
        if (strcmp(p->command, "BROADCAST") == 0)
        {
            char username[DEFAULT_BUFFER_SIZE];
            strcpy(username, p->parameters[0] + 5);
            fprintf(stdout, "From %s: %s\n", username, p->msg);
        }
        if (strcmp(p->command, "SEND-ROOM") == 0)
        {
            char room[DEFAULT_BUFFER_SIZE];
            char username[DEFAULT_BUFFER_SIZE];
            strcpy(room, p->parameters[0] + 5);
            strcpy(username, p->parameters[1] + 5);
            fprintf(stdout, "From %s@%s: %s\n", username, room, p->msg);
        }
    }
    free_payload(p);
}
