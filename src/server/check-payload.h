#ifndef CHECKPAYLOAD_H
#define CHECKPAYLOAD_H

#include "../payload.h"
#include "clients.h"
#include "rooms.h"
void check_payload(int epoll_instance, int client_socket,
                   struct clients *list_clients, struct rooms *r, char *ip);
void send_msg(int mode, int client_socket, char *cmd, char *msg);
void ping(int client_socket);
void login(struct payload_t *p, int client_socket,
           struct clients *list_clients);
void list_users(struct payload_t *p, int client_socket,
                struct clients *list_clients);
void broadcast(struct payload_t *p, struct client_item *sender,
               struct clients *list_clients);
void send_dm(struct payload_t *p, struct client_item *sender,
             struct clients *list_clients);
void create_room(struct rooms *r, struct payload_t *p, int client_socket);

void list_room(struct rooms *r, struct payload_t *p, int client_socket);
void join_room(struct rooms *r, struct payload_t *p,
               struct client_item *joiner);

void leave_room(struct rooms *r, struct payload_t *p,
                struct client_item *clients);
void send_room(struct rooms *list_room, struct payload_t *p,
               struct client_item *sender);
void profile(struct rooms *r, struct payload_t *t, struct client_item *client,
             char *ip);
void delete_room(struct rooms *r, struct payload_t *p,
                 struct client_item *clients);
#endif /* CHECKPAYLOAD_H  */
