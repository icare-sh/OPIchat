#define _DEFAULT_SOURCE

#include "payload.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct payload_t *init_payload(void)
{
    struct payload_t *new_payload = malloc(sizeof(struct payload_t));

    new_payload->size = 0;
    new_payload->status = 0;
    new_payload->command = NULL;
    new_payload->nb_parameters = 0;
    new_payload->parameters = NULL;
    new_payload->msg = NULL;
    new_payload->success = 0;

    return new_payload;
}

void free_payload(struct payload_t *p)
{
    if (p->command != NULL)
        free(p->command);
    for (int i = 0; i < p->nb_parameters; i++)
        free(p->parameters[i]);
    free(p->parameters);
    free(p->msg);
    free(p);
}

char *payload_to_txt(struct payload_t *payload)
{
    char status[3];
    char *txt = malloc(sizeof(char) * DEFAULT_BUFFER_SIZE);
    sprintf(txt, "%d\n", payload->size);
    sprintf(status, "%d\n", payload->status);
    strcat(txt, status);
    if (payload->command != NULL)
    {
        strcat(txt, payload->command);
        strcat(txt, "\n");
    }
    for (int i = 0; i < payload->nb_parameters; i++)
        if (payload->parameters[i] != NULL)
        {
            strcat(txt, payload->parameters[i]);
            strcat(txt, "\n");
        }
    strcat(txt, "\n");
    txt = realloc(txt, strlen(txt) + payload->size + 2);
    if (payload->msg != NULL && strlen(payload->msg) > 0)
    {
        strcat(txt, payload->msg);
        strcat(txt, "\n");
    }
    strcat(txt, "\0");
    return txt;
}

struct payload_t *set_payload(struct payload_t *p, int attr, char *token)
{
    char *test_int;
    if (attr == 0 || attr == 1)
    {
        int value = strtol(token, &test_int, 10);
        if (strlen(test_int) > 0 || strcmp(token, "") == 0)
        {
            p->success = -1;
            return p;
        }
        else if (attr == 0)
        {
            p->size = value;
            p->msg = malloc(p->size + 1);
            p->msg[0] = '\0';
        }
        else if (attr == 1)
            p->status = value;
    }
    else if (attr == 2)
    {
        p->command = malloc(sizeof(char) * strlen(token) + 1);
        sprintf(p->command, "%s", token);
    }
    else if (attr == 3)
    {
        p->parameters =
            realloc(p->parameters, sizeof(char *) * (p->nb_parameters + 2));
        p->parameters[p->nb_parameters] =
            malloc(sizeof(char) * strlen(token) + 1);
        sprintf(p->parameters[p->nb_parameters], "%s", token);
        p->nb_parameters += 1;
    }
    else if (attr == -1)
    {
        // AVANT : p->size = strlen(token) + 1;
        p->size = strlen(token) + 1;
        p->msg = malloc(p->size + 1);
        sprintf(p->msg, "%s", token);
    }
    else
    {
        if (strlen(p->msg) > 0 && p->size >= (int)(strlen(p->msg) + 1))
            strcat(p->msg, "\n");
        if (p->size >= (int)(strlen(p->msg) + strlen(token)))
            strcat(p->msg, token);
        else
            p->success = -1;
    }
    return p;
}

struct payload_t *txt_to_payload(char *txt)
{
    char *msg = malloc(strlen(txt) + 1);
    sprintf(msg, "%s", txt);
    struct payload_t *p = init_payload();
    char *sep = "\n";
    char *token = strsep(&msg, sep);
    int i = 0;
    while (token != NULL)
    {
        if (i != 3)
            p = set_payload(p, i, token);
        while (i == 3 && strlen(token) > 0)
        {
            p = set_payload(p, i, token);
            token = strsep(&msg, sep);
        }
        token = strsep(&msg, sep);
        i += 1;
    }
    free(msg);
    return p;
}

struct payload_t *set_msg_p(struct payload_t *p, char *token)
{
    p->size = strlen(token);
    free(p->msg);
    p->msg = NULL;
    p->msg = malloc(p->size + 1);
    sprintf(p->msg, "%s", token);
    return p;
}
