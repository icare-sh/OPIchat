#define _POSIX_C_SOURCE 200112L

#include "request.h"

#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../payload.h"

int check_cmd(char *cmd)
{
    char *valid_cmd[12] = { "PING",       "LOGIN",       "LIST-USERS",
                            "SEND-DM",    "BROADCAST",   "CREATE-ROOM",
                            "LIST-ROOMS", "JOIN-ROOM",   "LEAVE-ROOM",
                            "SEND-ROOM",  "DELETE-ROOM", "PROFILE" };
    for (int i = 0; i < 12; i++)
    {
        if (strcmp(valid_cmd[i], cmd) == 0)
            return 0;
    }
    return -1;
}

int cmd_need_param(char *cmd)
{
    char *valid_cmd[2] = { "SEND-DM", "SEND-ROOM" };
    for (int i = 0; i < 2; i++)
    {
        if (strcmp(valid_cmd[i], cmd) == 0)
            return 0;
    }
    return -1;
}

int cmd_always_payload(char *cmd)
{
    char *valid_cmd[3] = { "SEND-DM", "SEND-ROOM", "BROADCAST" };
    for (int i = 0; i < 3; i++)
    {
        if (strcmp(valid_cmd[i], cmd) == 0)
            return 0;
    }
    return -1;
}

char *read_stdin()
{
    char buffer[2];
    char *msg = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    msg[0] = '\0';
    while (1)
    {
        char *rec = fgets(buffer, 2, stdin);
        if (rec == NULL)
            exit(1);
        if (strchr(buffer, '\n') != NULL)
            break;
        strcat(msg, buffer);
    }
    return msg;
}

void get_command(struct payload_t *p)
{
    while (1)
    {
        fprintf(stdout, "Command:\n");
        char *txt = read_stdin();
        if (check_cmd(txt) == 0)
        {
            p = set_payload(p, 2, txt);
            free(txt);
            break;
        }
        else
        {
            fprintf(stderr, "Invalid command\n");
            free(txt);
        }
    }
}

void get_param(struct payload_t *p) // a traiter
{
    regex_t regex;
    int reti;
    while (1)
    {
        char *txt = read_stdin();
        if (strlen(txt) == 0)
        {
            free(txt);
            break;
        }
        reti = regcomp(&regex, "^[a-zA-Z0-9]+=[a-zA-Z0-9]+$", REG_EXTENDED);
        reti = regexec(&regex, txt, 0, NULL, 0);
        if (!reti)
        {
            p = set_payload(p, 3, txt);
            free(txt);
        }
        else
            fprintf(stderr, "Invalid parameter\n");
        regfree(&regex);
    }
}

void get_payload(struct payload_t *p)
{
    fprintf(stdout, "Payload:\n");
    char *txt = read_stdin();
    p = set_msg_p(p, txt);
    free(txt);
}

void request(int socket_server)
{
    struct payload_t *p = init_payload();
    char *txt;
    get_command(p);

    if (cmd_need_param(p->command) == 0)
    {
        fprintf(stdout, "Parameters:\n");
        get_param(p);
    }
    if (cmd_always_payload(p->command) == 0)
    {
        while (1)
        {
            get_payload(p);
            txt = payload_to_txt(p);
            if (strcmp(p->msg, "/quit") == 0)
                break;
            if (send(socket_server, txt, strlen(txt), 0) == -1)
                perror("ERROR SEND");
        }
    }
    else
    {
        get_payload(p);
        txt = payload_to_txt(p);
        // printf("size : '%d'\n", p->size);
        // printf("status : '%d'\n", p->status);
        // printf("command : '%s'\n", p->command);
        // printf("msg : '%s'\n", p->msg);
        if (send(socket_server, txt, strlen(txt), 0) == -1)
            perror("ERROR SEND");
    }
    free_payload(p);
    free(txt);
}
