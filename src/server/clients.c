#include "clients.h"

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rooms.h"

struct clients *clients_init(void)
{
    struct clients *c = malloc(sizeof(struct clients));
    c->size = 0;
    c->head = NULL;
    c->tail = NULL;
    return c;
}

void clients_print(const struct clients *list)
{
    if (list != NULL)
    {
        size_t n = list->size;
        struct client_item *item = list->head;
        printf("NB_CLIENT : %ld\n", n);
        for (size_t i = 0; i < n; i++)
        {
            printf("socket : %d\n", item->socket);
            // if (item->username != NULL && strlen(item->username) > 0)
            //    printf("username : %s\n", item->username);
            // if (item->msg != NULL && strlen(item->msg) > 0)
            //    printf("msg : %s\n", item->msg);
            item = item->next;
        }
    }
}

void clients_push(struct clients *list, int socket)
{
    if (socket < 0 || list == NULL)
        errx(1, "Failed to add socket");
    struct client_item *item = malloc(sizeof(struct client_item));
    item->socket = socket;
    item->username = NULL;
    item->msg = NULL;
    item->next = NULL;
    item->prev = list->tail;
    if (list->tail == NULL)
        list->head = item;
    else
        list->tail->next = item;
    list->tail = item;
    list->size += 1;
}

struct client_item *client_get_by_socket(struct clients *list, int socket)
{
    if (list == NULL)
        return NULL;
    struct client_item *item = list->head;
    for (size_t i = 0; i < list->size; i++)
    {
        if (item->socket == socket)
            return item;
        item = item->next;
    }
    return NULL;
}

struct client_item *client_get_by_username(struct clients *list, char *username)
{
    if (list == NULL)
        return NULL;
    struct client_item *item = list->head;
    for (size_t i = 0; i < list->size; i++)
    {
        if (item->username && strcmp(item->username, username) == 0)
            return item;
        item = item->next;
    }
    return NULL;
}

int client_set_username(struct clients *list, struct client_item *c,
                        char *username)
{
    if (strlen(username) == 0)
        return -1;
    for (size_t i = 0; i < strlen(username); i++)
    {
        if (!(isalnum(username[i])))
            return -1;
    }
    struct client_item *item = client_get_by_username(list, username);
    if (item != NULL)
        return 0;
    c->username = malloc(strlen(username) + 1);
    strcpy(c->username, username);
    return 1;
}

void client_set_msg(struct client_item *c, char *msg)
{
    c->msg = malloc(strlen(msg) + 1);
    strcpy(c->msg, msg);
}

void clients_remove_index(struct clients *list, size_t index)
{
    if (list == NULL || index >= list->size)
        errx(1, "Failed to remove index");
    struct client_item *item = list->head;
    for (size_t i = 0; i < index; i++)
        item = item->next;
    if (close(item->socket) == -1)
        errx(1, "Failed to close socket");
    struct client_item *avant = item->prev;
    struct client_item *apres = item->next;
    if (index == 0 || index == list->size - 1)
    {
        if (index == 0)
        {
            list->head = apres;
            if (apres != NULL)
                apres->prev = NULL;
        }
        else
        {
            list->tail = avant;
            avant->next = NULL;
        }
    }
    else
    {
        avant->next = apres;
        apres->prev = avant;
    }
    free(item->username);
    free(item->msg);
    free(item);
    list->size -= 1;
}

int clients_remove_socket(struct clients *list, int socket,
                          struct rooms *list_rooms)
{
    if (list == NULL || socket < 0 || list->size == 0)
        return 0;
    int i = 0;
    struct client_item *item = list->head;
    while (item != NULL)
    {
        if (item->socket == socket)
        {
            struct room *r = list_rooms->head;
            for (size_t i = 0; i < list_rooms->size; i++)
            {
                struct room *next = r->next;
                if (find_client_in_room(r, socket) != -1)
                    del_client_in_room(r, socket);
                if (r->owner == socket)
                    del_room_by_name(list_rooms, r->name);
                r = next;
            }
            clients_remove_index(list, i);
            fprintf(stdout, "Client disconnected\n");
            return 1;
        }
        item = item->next;
        i++;
    }
    return 0;
}

void clients_clear(struct clients *list)
{
    while (list != NULL && list->size > 0)
        clients_remove_index(list, 0);
    free(list);
}
