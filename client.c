#include <stdio.h>
#include "client.h"
#include "log.h"

void clients_info_init(struct clients_info *info, int max_clients) {
    memset(info, 0, sizeof(*info));
    info->max_clients = max_clients;
}

struct client *new_add_client(struct clients_info *info, int fd) {
    if (info->nr_clients >= info->max_clients) {
        LOG_ERR(">>>> too more clients");
        return NULL;
    }

    struct client *p = (struct client *)malloc(sizeof(struct client));
    if (!p) {
        LOG_ERR("malloc client failed");
        return NULL;
    }

    memset(p, 0, sizeof(*p));
    p->fd = fd;
    p->last_active_time = time(NULL);
    if (!info->nr_clients) {
        info->nr_clients = 1;
        info->head = p;
        info->tail = p;
    } else {
        info->nr_clients++;
        info->tail->next = p;
        struct client *tmp_tail = info->tail;
        info->tail = p; 
        p->prev = tmp_tail;
    }
}

void del_client(struct clients_info *info, struct client *client) {

    if (client == info->head) {
        LOG_DEBUG("delete client head");
        info->head = client->next;
        if (client->next)
            client->next->prev = 0;
    } else if(client == info->tail) {
        LOG_DEBUG("delete client tail");
        info->tail = client->prev;
        if (client->prev)
            client->prev->next = 0;
    } else {
        LOG_DEBUG("delete client");
        client->prev->next = client->next;
        client->next->prev = client->prev;
    }

    info->nr_clients--;

    if (!info->nr_clients) {
        info->head = 0;
        info->tail = 0;
    }
    LOG_INFO(">>> delete client %p now remain clients = %d", client, info->nr_clients);
    free(client);
}
