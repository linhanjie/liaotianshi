#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "user.h"

struct client;

struct clients_info {
    int nr_clients;
    int max_clients;
    struct client *head;
    struct client *tail;
};

struct client {
    user_t *user;

    int fd;
    int id;
    char name[100];
    int status;
    struct client *prev;
    struct client *next;
};

void clients_info_init(struct clients_info *info, int max_fd);

struct client *add_client(struct clients_info *info, int fd);

void del_client(struct clients_info *info, struct client *client);

#define for_each_client(clients_info, p) \
    for(p=(clients_info)->head; p; p=p->next)

#define for_each_client_safe(clients_info, p, n) \
    for (p = (clients_info)->head,  \
         n = (p ? p->next : NULL); \
         p; \
         p = n, \
         n = (p ? p->next : NULL))
#endif
