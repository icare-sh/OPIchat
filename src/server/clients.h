#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>

#include "rooms.h"

struct client_item
{
    int socket;
    char *username;
    char *msg;
    struct client_item *next;
    struct client_item *prev;
};

struct clients
{
    size_t size;
    struct client_item *head;
    struct client_item *tail;
};

// Client 1
struct clients *clients_init(void);
void clients_print(const struct clients *list);
void clients_push(struct clients *list, int socket);
struct client_item *client_get_by_socket(struct clients *list, int socket);
struct client_item *client_get_by_username(struct clients *list,
                                           char *username);

// Client 2
int client_set_username(struct clients *list, struct client_item *c,
                        char *username);
void client_set_msg(struct client_item *c, char *msg);
void clients_remove_index(struct clients *list, size_t index);
int clients_remove_socket(struct clients *list, int socket,
                          struct rooms *list_rooms);
void clients_clear(struct clients *list);

#endif // CLIENT_H
