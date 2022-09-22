#ifndef PAYLOAD_H
#define PAYLOAD_H

#define DEFAULT_BUFFER_SIZE 2048

struct payload_t
{
    int size;
    int status;
    char *command;
    int nb_parameters;
    char **parameters;
    char *msg;
    int success;
};

struct payload_t *init_payload(void);
void free_payload(struct payload_t *p);
char *payload_to_txt(struct payload_t *payload);
struct payload_t *set_payload(struct payload_t *p, int attr, char *token);
struct payload_t *txt_to_payload(char *txt);
struct payload_t *set_msg_p(struct payload_t *p, char *token);

#endif /* PAYLOAD_H */
