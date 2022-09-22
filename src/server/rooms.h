#ifndef ROOM_H
#define ROOM_H

#include <sys/types.h>

struct room
{
    char *name;
    int owner;
    size_t nb_client;
    int *clients;
    struct room *next;
    struct room *prev;
};

struct rooms
{
    size_t size;
    struct room *head;
    struct room *tail;
};

struct rooms *rooms_init(void);
char *get_list_rooms(struct rooms *list_rooms);
int room_push(struct rooms *list_rooms, char *name, int owner);
struct room *room_get_by_name(struct rooms *list_rooms, char *name);
void del_room_by_index(struct rooms *list, size_t index);
int del_room_by_name(struct rooms *list, char *name);

void add_client_in_room(struct room *r, int new_client);
void del_client_in_room(struct room *r, int client);
int find_client_in_room(struct room *r, int client);

#endif // ROOM_H
