#ifndef REQUEST_H
#define REQUEST_H

#include <err.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../payload.h"
#include "request.h"

int check_cmd(char *cmd);
int cmd_need_param(char *cmd);
int cmd_always_payload(char *cmd);

char *read_stdin();
void get_command(struct payload_t *p);
void get_param(struct payload_t *p);
void get_payload(struct payload_t *p);

void request(int socket_server);

#endif
