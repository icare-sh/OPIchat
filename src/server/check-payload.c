#include "check-payload.h"

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "clients.h"
#include "rooms.h"
void check_payload(int epoll_instance, int client_socket,
                   struct clients *list_clients, struct rooms *r, char *ip)
{
    (void)epoll_instance;

    struct client_item *client =
        client_get_by_socket(list_clients, client_socket);
    struct payload_t *p = txt_to_payload(client->msg);
    if (p->success != 0)
    {
        free_payload(p);
        free(r);
        send_msg(3, client_socket, "INVALID", "Bad request");
        epoll_ctl(epoll_instance, EPOLL_CTL_DEL, client_socket, NULL);
        clients_remove_socket(list_clients, client_socket, r);
        return;
    }
    printf("Client data\n< Request in:\n%s", client->msg);
    if (strcmp(p->command, "PING") == 0)
        send_msg(1, client_socket, "PING", "PONG");
    else if (strcmp(p->command, "LOGIN") == 0)
        login(p, client_socket, list_clients);
    else if (strcmp(p->command, "LIST-USERS") == 0)
        list_users(p, client_socket, list_clients);
    else if (strcmp(p->command, "SEND-DM") == 0)
        send_dm(p, client, list_clients);
    else if (strcmp(p->command, "BROADCAST") == 0)
        broadcast(p, client, list_clients);
    else if (strcmp(p->command, "CREATE-ROOM") == 0)
        create_room(r, p, client_socket);
    else if (strcmp(p->command, "LIST-ROOMS") == 0)
        list_room(r, p, client_socket);
    else if (strcmp(p->command, "JOIN-ROOM") == 0)
        join_room(r, p, client);
    else if (strcmp(p->command, "SEND-ROOM") == 0)
        send_room(r, p, client);
    else if (strcmp(p->command, "LEAVE-ROOM") == 0)
        leave_room(r, p, client);
    else if (strcmp(p->command, "DELETE-ROOM") == 0)
        delete_room(r, p, client);
    else if (strcmp(p->command, "PROFILE") == 0)
        profile(r, p, client, ip);

    free_payload(p);
    // free(r);
}

void send_msg(int mode, int client_socket, char *cmd, char *msg)
{
    struct payload_t *p = init_payload();
    p->status = mode;
    p = set_payload(p, 2, cmd);
    if (strcmp(msg, "") == 0)
    {
        p->size = 0;
        p->msg = NULL;
    }
    else
        p = set_payload(p, -1, msg);
    char *txt = payload_to_txt(p);
    send(client_socket, txt, strlen(txt), MSG_NOSIGNAL);
    printf("< Messsage out:\n%s", txt);
    free_payload(p);
    free(txt);
}

void send_msg_with_param(int mode, int client_socket, char *cmd, char *param,
                         char *msg)
{
    struct payload_t *p = init_payload();
    p->status = mode;
    p = set_payload(p, 2, cmd);
    p = set_payload(p, 3, param);
    if (strcmp(msg, "") == 0)
    {
        p->size = 0;
        p->msg = NULL;
    }
    else
        p = set_payload(p, -1, msg);
    char *txt = payload_to_txt(p);
    send(client_socket, txt, strlen(txt), MSG_NOSIGNAL);
    printf("< Messsage out:\n%s", txt);
    free_payload(p);
    free(txt);
}

void login(struct payload_t *p, int client_socket, struct clients *list_clients)
{
    struct client_item *client =
        client_get_by_socket(list_clients, client_socket);
    int req = client_set_username(list_clients, client, p->msg);
    if (req == -1)
        send_msg(3, client_socket, "LOGIN", "Bad username");
    else if (req == 0)
        send_msg(3, client_socket, "LOGIN", "Duplicate username");
    else
    {
        p->status = 1;
        p = set_payload(p, -1, "Logged in");
        char *txt = payload_to_txt(p);
        send(client_socket, txt, strlen(txt), MSG_NOSIGNAL);
        printf("< Message out:\n%s", txt);
        free(txt);
    }
}

void list_users(struct payload_t *p, int client_socket,
                struct clients *list_clients)
{
    struct client_item *client = list_clients->head;
    char list[DEFAULT_BUFFER_SIZE];
    list[0] = '\0';
    for (size_t i = 0; i < list_clients->size; i++)
    {
        if (client->username != NULL && strlen(client->username) > 0)
        {
            if (strlen(list) > 0)
                strcat(list, "\n");
            strcat(list, client->username);
        }
        client = client->next;
    }
    if (list[0] == '\0')
    {
        p->size = 0;
        p->msg = NULL;
    }
    else
        p = set_payload(p, -1, list);
    p->status = 1;
    char *txt = payload_to_txt(p);
    send(client_socket, txt, strlen(txt), MSG_NOSIGNAL);
    printf("< Message out:\n%s", txt);
    free(txt);
}

void broadcast(struct payload_t *p, struct client_item *sender,
               struct clients *list_clients)
{
    struct client_item *client = list_clients->head;
    p->status = 2;
    char *from = NULL;
    if (sender->username == NULL)
    {
        from = malloc(17);
        sprintf(from, "From=<Anonymous>");
    }
    else
    {
        from = malloc(strlen(sender->username) + 6);
        sprintf(from, "From=%s", sender->username);
    }
    p = set_payload(p, 3, from);
    char *txt = payload_to_txt(p);
    for (size_t i = 0; i < list_clients->size; i++)
    {
        if (client->socket != sender->socket)
            send(client->socket, txt, strlen(txt), MSG_NOSIGNAL);
        client = client->next;
    }
    send_msg(1, sender->socket, "BROADCAST", "");
    printf("< Message out:\n%s", txt);
    free(txt);
    free(from);
}

void send_dm(struct payload_t *p, struct client_item *sender,
             struct clients *list_clients)
{
    struct client_item *receiver =
        client_get_by_username(list_clients, p->parameters[0] + 5);
    if (strncmp(p->parameters[0], "User=", 5) != 0 || p->nb_parameters > 1)
        send_msg_with_param(3, sender->socket, "SEND-DM", p->parameters[0],
                            "Missing parameter");
    else if (receiver == NULL)
        send_msg_with_param(3, sender->socket, "SEND-DM", p->parameters[0],
                            "User not found");
    else
    {
        char *from = NULL;
        if (sender->username == NULL)
        {
            from = malloc(17);
            sprintf(from, "From=<Anonymous>");
        }
        else
        {
            from = malloc(strlen(sender->username) + 6);
            sprintf(from, "From=%s", sender->username);
        }
        p->status = 2;
        p = set_payload(p, 3, from);
        if (strlen(p->msg) == 0)
            p = set_payload(p, -1, "");
        char *txt = payload_to_txt(p);
        send(receiver->socket, txt, strlen(txt), MSG_NOSIGNAL);
        printf("< Message out:\n%s", txt);
        free(txt);
        free(from);
        if (sender->socket != receiver->socket)
            send_msg_with_param(1, sender->socket, "SEND-DM", p->parameters[0],
                                "");
    }
}

void create_room(struct rooms *r, struct payload_t *p, int client_socket)
{
    int req = room_push(r, p->msg, client_socket);
    if (req == -1)
        send_msg(3, client_socket, "CREATE-ROOM", "Bad room name");
    else if (req == 0)
        send_msg(3, client_socket, "CREATE-ROOM", "Duplicate room name");
    else
    {
        p->status = 1;
        p = set_payload(p, -1, "Room created");
        char *txt = payload_to_txt(p);
        send(client_socket, txt, strlen(txt), MSG_NOSIGNAL);
        printf("< Message out:\n%s", txt);
        free(txt);
    }
}

void list_room(struct rooms *r, struct payload_t *p, int client_socket)

{
    char *list_des_rooms = get_list_rooms(r);
    if (list_des_rooms[0] == '\0')
    {
        p->size = 0;
        p->msg = NULL;
    }
    else
        p = set_payload(p, -1, list_des_rooms);
    /*
    char list[DEFAULT_BUFFER_SIZE];
    list[0] = '\0';
    struct room *rm = r->head;
    for (size_t i = 0; i < r->size; i++)
    {
        strcat(list, rm->name);
        strcat(list, "\n");
        rm = rm->next;
    }
    list[strlen(list) - 1] = '\0';
    if (list[0] == '\0')
    {
        p->size = 0;
        p->msg = NULL;
    }
    else
        p = set_payload(p, -1, list);*/

    p->status = 1;
    char *txt = payload_to_txt(p);
    send(client_socket, txt, strlen(txt), MSG_NOSIGNAL);
    printf("< Message out:\n%s", txt);
    free(list_des_rooms);
    free(txt);
}

void join_room(struct rooms *r, struct payload_t *p, struct client_item *joiner)
{
    struct room *rm = room_get_by_name(r, p->msg);

    if (rm == NULL)
        send_msg(3, joiner->socket, "JOIN-ROOM", "Room not found");
    else
    {
        rm->clients[rm->nb_client] = joiner->socket;
        rm->nb_client += 1;
        rm->clients =
            realloc(rm->clients, sizeof(size_t) * (rm->nb_client + 1));

        p->status = 1;
        p = set_payload(p, -1, "Room joined");
        char *txt = payload_to_txt(p);
        send(joiner->socket, txt, strlen(txt), MSG_NOSIGNAL);
        printf("< Message out:\n%s", txt);
        free(txt);
    }
}

void leave_room(struct rooms *r, struct payload_t *p,
                struct client_item *client)
{
    struct room *rm = room_get_by_name(r, p->msg);

    if (rm == NULL)
        send_msg(3, client->socket, "LEAVE-ROOM", "Room not found");
    else
    {
        int res = find_client_in_room(rm, client->socket);
        // if (res == -1)
        //{
        //    return;
        //}
        if (res != -1)
            del_client_in_room(rm, client->socket);
        p->status = 1;
        p = set_payload(p, -1, "Room left");
        char *txt = payload_to_txt(p);
        send(client->socket, txt, strlen(txt), MSG_NOSIGNAL);
        printf("< Message out:\n%s", txt);
        free(txt);
    }
}

void send_room(struct rooms *list_room, struct payload_t *p,
               struct client_item *sender)
{
    struct room *r = room_get_by_name(list_room, p->parameters[0] + 5);
    if (strncmp(p->parameters[0], "Room=", 5) != 0 || p->nb_parameters > 1)
        send_msg_with_param(3, sender->socket, "SEND-ROOM", p->parameters[0],
                            "Missing parameter");
    else if (r == NULL)
        send_msg_with_param(3, sender->socket, "SEND-ROOM", p->parameters[0],
                            "Room not found");
    else
    {
        char *from = NULL;
        if (sender->username == NULL)
        {
            from = malloc(17);
            sprintf(from, "From=<Anonymous>");
        }
        else
        {
            from = malloc(strlen(sender->username) + 6);
            sprintf(from, "From=%s", sender->username);
        }
        p->status = 2;
        p = set_payload(p, 3, from);
        if (strlen(p->msg) == 0)
            p = set_payload(p, -1, "");
        char *txt = payload_to_txt(p);
        for (size_t i = 0; i < r->nb_client; i++)
        {
            if (r->clients[i] != sender->socket)
                send(r->clients[i], txt, strlen(txt), MSG_NOSIGNAL);
        }
        printf("< Message out:\n%s", txt);
        free(txt);
        free(from);
        send_msg_with_param(1, sender->socket, "SEND-ROOM", p->parameters[0],
                            "");
    }
}

void delete_room(struct rooms *r, struct payload_t *p,
                 struct client_item *client)
{
    struct room *rm = room_get_by_name(r, p->msg);

    if (rm == NULL)
        send_msg(3, client->socket, "DELETE-ROOM", "Room not found");
    else
    {
        if (rm->owner != client->socket)
            send_msg(3, client->socket, "DELETE-ROOM", "Unauthorized");
        else
        {
            del_room_by_name(r, p->msg);

            p->status = 1;
            p = set_payload(p, -1, "Room deleted");
            char *txt = payload_to_txt(p);
            send(client->socket, txt, strlen(txt), MSG_NOSIGNAL);
            printf("< Message out:\n%s", txt);
            free(txt);
        }
    }
}

void profile(struct rooms *r, struct payload_t *p, struct client_item *client,
             char *ip)
{
    char list[DEFAULT_BUFFER_SIZE];

    list[0] = '\0';
    strcat(list, "Username: ");
    if (client->username != NULL)
    {
        strcat(list, client->username);
        strcat(list, "\n");
    }
    else
    {
        strcat(list, "<Anonymous>");
        strcat(list, "\n");
    }
    strcat(list, "IP: ");
    strcat(list, ip);

    strcat(list, "\n");
    strcat(list, "Rooms:");
    struct room *item = r->head;
    while (item != NULL)
    {
        if (find_client_in_room(item, client->socket) != -1)
        {
            strcat(list, "\n");
            strcat(list, item->name);
        }
        item = item->next;
    }

    p->status = 1;
    p = set_payload(p, -1, list);
    char *txt = payload_to_txt(p);
    send(client->socket, txt, strlen(txt), MSG_NOSIGNAL);
    printf("< Message out:\n%s", txt);
    free(txt);
}
