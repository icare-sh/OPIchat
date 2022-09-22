#include "rooms.h"

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "clients.h"
#define DEFAULT_BUFFER_SIZE 1024
struct rooms *rooms_init(void)
{
    struct rooms *r = malloc(sizeof(struct rooms));
    r->size = 0;
    r->head = NULL;
    r->tail = NULL;
    return r;
}

char *get_list_rooms(struct rooms *list_rooms)
{
    char *list = malloc(DEFAULT_BUFFER_SIZE);
    list[0] = '\0';
    size_t n = list_rooms->size;

    struct room *r = list_rooms->head;

    for (size_t i = 0; i < n; i++)

    {
        if (strlen(list) > 0)
            strcat(list, "\n");
        strcat(list, r->name);
        r = r->next;
    }
    list[strlen(list)] = '\0';

    return list;
}

int room_push(struct rooms *list_rooms, char *name, int owner)
{
    if (strlen(name) == 0)
        return -1;
    for (size_t i = 0; i < strlen(name); i++)
    {
        if (!(isalnum(name[i])))
            return -1;
    }
    struct room *duplicate = room_get_by_name(list_rooms, name);
    if (duplicate != NULL)
        return 0;
    struct room *r = malloc(sizeof(struct room));
    r->nb_client = 0;
    r->owner = owner;
    r->name = malloc(strlen(name) + 1);
    strcpy(r->name, name);
    r->clients = malloc(sizeof(int));
    r->prev = list_rooms->tail;
    r->next = NULL;
    if (list_rooms->tail == NULL)
        list_rooms->head = r;
    else
        list_rooms->tail->next = r;
    list_rooms->tail = r;
    list_rooms->size += 1;

    return 1;
}

struct room *room_get_by_name(struct rooms *list_rooms, char *name)
{
    if (list_rooms == NULL)
        return NULL;
    struct room *r = list_rooms->head;
    for (size_t i = 0; i < list_rooms->size; i++)
    {
        if (strcmp(r->name, name) == 0)
            return r;
        r = r->next;
    }
    return NULL;
}

void del_room_by_index(struct rooms *list, size_t index)
{
    if (list == NULL || index >= list->size)
        errx(1, "Failed to remove index");
    struct room *item = list->head;
    for (size_t i = 0; i < index; i++)
        item = item->next;
    struct room *avant = item->prev;
    struct room *apres = item->next;

    if (index == 0 || index == list->size - 1)
    {
        if (index == 0)
        {
            list->head = apres;

            if (apres != NULL)
            {
                apres->prev = NULL;
            }
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

    free(item->name);
    free(item->clients);
    free(item);

    list->size -= 1;
}

int del_room_by_name(struct rooms *list, char *name)
{
    if (list == NULL || list->size == 0)
        return 0;
    int i = 0;
    struct room *item = list->head;
    while (item != NULL)
    {
        if (strcmp(item->name, name) == 0)
        {
            del_room_by_index(list, i);
            return 1;
        }
        item = item->next;
        i++;
    }
    return 0;
}

void add_client_in_room(struct room *r, int new_client)
{
    r->clients[r->nb_client] = new_client;
    r->nb_client += 1;
    r->clients = realloc(r->clients, sizeof(int) * (r->nb_client + 1));
}

int find_client_in_room(struct room *r, int client)
{
    for (size_t i = 0; i < r->nb_client; i++)
    {
        if (r->clients[i] == client)
            return i;
    }
    return -1;
}

void del_client_in_room(struct room *r, int client)
{
    int i = find_client_in_room(r, client);
    int pos;
    for (pos = i; pos < (int)r->nb_client - 1; pos++)
    {
        r->clients[pos] = r->clients[pos + 1];
    }
    r->nb_client += 1;
}

/*int main()
{
    // struct room *r;
    struct rooms *l;
    char buff1[40];
    char buff2[40];
    int sck = 0;
    printf("Donnez le nom de la room: \n");
    scanf("%s", buff1);

    scanf("%s", buff2);
    printf("Donnez le sck de le owner: \n");
    scanf("%d", &sck);

    l = rooms_init();
    room_push(l, buff1, sck);
    room_push(l, buff2, sck);
    char *list = get_list_rooms(l);
    printf("list rooms \n'%s'", list);

    del_room_by_name(l, buff1);

    del_room_by_name(l, buff2);
    free(list);

    free(l);
    return 0;
}*/
