#include <stdio.h>
#include <errno.h>
#include "request.h"

struct request *read_request(struct client *p) {
    int ret;
    struct request *rq = (struct request*)malloc(sizeof(struct request));
    if (!rq) {
        printf("can't malloc request\n");
        return NULL;
    }

    rq->p = p;

    int request_head_size = (sizeof(*rq)-sizeof(rq->p)); 
    ret = recv(p->fd, &rq->version, request_head_size, 0);
    if (ret <= 0) {        // client close
        printf("read client %d ret = %d\n", p->fd, ret);
        //printf("read client[%d] erro ret = %d[%s]close\n", p->fd, ret, strerror(errno));
        close(p->fd);
        return NULL;
    } else if (ret < request_head_size) {
        printf("read head failed, ret = %d\n", ret);
    } 

    printf("new request: %p\n", rq);
    return rq;
}

int do_request(struct request *rq) {
    switch(rq->type) {
    case RQ_REGISTER_TYPE:
        do_register_request(rq);
        break;
    case RQ_LOGIN_TYPE:
        do_login_request(rq);
        break;
    case RQ_LOGOUT_TYPE:
        do_logout_request(rq);
        break;
    case RQ_SHOW_ACTIVE_USERS_TYPE:
        do_show_active_users_request(rq);
        break;
    case RQ_SND_MSG_TYPE:
        do_snd_msg_request(rq);
        break;
    case RQ_SND_MSG_ALL_TYPE:
        do_snd_msg_all_request(rq);
        break;
    case RQ_HEART_BEAT_TYPE:
        do_heart_beat_request(rq);
        break;
    default:
        printf("error type %d\n", rq->type);
        return 1;
    }

    return 0;
}


int send_response(client_t *client, int ret, int have_msg, char *msg) {

    response_t resp;
    resp.ret = ret;
    resp.have_msg = have_msg;
    memset(resp.msg, 0, sizeof(resp.msg));
    
    if (have_msg)
        strcpy(resp.msg, msg);

    int left = sizeof(resp);
    char *p = &resp;
    while (left) 
    {
        int n = write(client->fd, p, left);
        if (n == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("send respone");
                return 1;
            }
        } 
            
        left -= n;
        p += n;
    }

    return 0;
}


void do_login_request(request_t *rq) {
    printf("login request: %s,%s\n", rq->from, rq->to);
  
    user_t *user = search_user(rq->from);
    if (!user) {
        send_response(rq->p, 1, 1, "user doesn't exist");
    } else {
        if (!strcmp(user->passwd, rq->to)) {
            send_response(rq->p, 0, 1, "login success");
            
            //login success
            rq->p->user = user;
        }
        else 
            send_response(rq->p, 1, 1, "passwd error");
    }

}

void do_register_request(request_t *rq) {

}

extern struct clients_info clients_info;

void do_show_active_users_request(request_t *rq) {
   
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    char *p = buf;


    client_t *client;
    for_each_client(&clients_info, client) {
        if (client->user) {
            printf("active user: %s\n", client->user->name);
            sprintf(p, "%s ", client->user->name);
            p += strlen(client->user->name) + 1;
        }
    }
    send_response(rq->p, 0, 1, buf);
}

void do_snd_msg_request(request_t *rq) {
}

void do_snd_msg_all_request(request_t *rq) {
}

void do_heart_beat_request(request_t *rq) {
}

void do_logout_request(request_t *rq) {
}

