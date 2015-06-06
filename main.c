/* server.c */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>


#include "client.h"
#include "work_thread.h"
#include "request.h"
#include "user.h"
#include "log.h"

static void show_client(struct sockaddr_in client_addr) {
    char ip_addr[100];
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_addr, sizeof(ip_addr));
    LOG_INFO("client connect: %s:%d", ip_addr, ntohs(client_addr.sin_port));
}

#define MAX_CLIENTS 100
struct clients_info clients_info;
struct message_queue queue;
hash_table_t *table;


void * heart_beat_thread(void * data) {
    while (1)
    {
        struct timeval tv = {10, 0}; /* 20 seconds*/

        if (-1 == select(0, NULL, NULL, NULL, &tv))
        {
            if (EINTR == errno)
            {
                continue; 
            }

            perror("select()");
            exit(1);
        }
        LOG_DEBUG(">>>> check inactive clients...");
        client_t *client;
        client_t *n;
        for_each_client_safe(&clients_info, client, n) {
            time_t now = time(NULL);
            if ((now - client->last_active_time) > 10) {
               LOG_INFO("client %d, %s is inactive", client->fd, client->user ? client->user->name:NULL);
                close(client->fd);
                del_client(&clients_info, client);
            }
        }
    }
}



int main() {
    
    LOG_ERR("hshshhsh");
    LOG_WARNING("hshshhsh");
    LOG_INFO("hshshhsh");
    LOG_DEBUG("hshshhsh");


    int sock_fd,new_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in client_addr;
    char buff[100];
    int numbytes;


    table = load_users_from_file("users.db");

    clients_info_init(&clients_info, MAX_CLIENTS);
    message_queue_init(&queue);
    work_threads_init(&queue);

    //建立TCP套接口
    if((sock_fd = socket(AF_INET,SOCK_STREAM,0))==-1) {
        perror("socket");
        exit(1);
    }
    //初始化结构体，并绑定2323端口
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(2323);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero),8);

    int opt = 1;
    setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    //绑定套接口
    if(bind(sock_fd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))==-1)
    {
        perror("bind");
        exit(1);
    }
    //创建监听套接口
    if(listen(sock_fd,10)==-1) {
        perror("listen");
        exit(1);
    }
    LOG_INFO("server is run...");

    pthread_t tid;

    pthread_create(&tid,NULL, heart_beat_thread,  NULL);


    fd_set fdsr;
    int max_fd;
    struct timeval tv;

    int sin_size = sizeof(client_addr);

    while (1) {
        // initialize file descriptor set
        FD_ZERO(&fdsr);
        FD_SET(sock_fd, &fdsr);
        max_fd = sock_fd;

        // add active connection to fd set
        struct client *p;
        struct client *n;
        for_each_client_safe(&clients_info, p, n) {
            FD_SET(p->fd, &fdsr);
            max_fd = max_fd > p->fd ?  max_fd : p->fd;
        }

        LOG_DEBUG(">>>> select start...");
        int ret = select(max_fd + 1, &fdsr, NULL, NULL, NULL);
        if (ret < 0) {
            perror("select");
            break;
        } else if (ret == 0) {
            LOG_ERR("timeout");
            continue;
        }

        // check whether a new connection comes
        if (FD_ISSET(sock_fd, &fdsr)) {
            new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
            if (new_fd <= 0) {
                perror("accept");
                continue;
            }
            show_client(client_addr);
            new_add_client(&clients_info, new_fd);
        }

        // check every fd in the set
        for_each_client_safe(&clients_info, p, n) {
            if (FD_ISSET(p->fd, &fdsr)) {
                struct request *rq = read_request(p);
                if (rq) {
                    enqueue_message(&queue, rq);
                } else {
                    del_client(&clients_info, p);
                }
            }
        }
    }

    close(sock_fd);

    return 0;
}

