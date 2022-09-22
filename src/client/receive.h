#ifndef RECEIVE_H
#define RECEIVE_H

char *get_username(char *txt);
char *get_room(char *txt, int user);
void receive(int server_socket);

#endif
