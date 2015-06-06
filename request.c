#include <stdio.h>
#include <errno.h>
#include "request.h"

extern struct clients_info clients_info;

struct request *read_request(struct client *client) {
    LOG_DEBUG("%s() start", __func__);
    int ret;
    request_t head;

    char *p = &head.version;
    //int request_head_size = (sizeof(head) - sizeof(head.p)); 
    int left = REQUEST_HEAD_SIZE;
    int n;
    while (left) {
        n = read(client->fd, p, left);
        if (n == -1) {
            if (errno == EINTR)
                continue;

            LOG_ERR("read error : %d", n);
            return NULL;
        } else if (n == 0) {
            LOG_ERR("client close");
            close(client->fd);
            return NULL;
        }

        left -= n;
        p += n;
    }
    LOG_DEBUG("read_request: type = %d, from = %s, to = %s, body_size = %d", head.type, head.from, head.to, head.body_size);

    head.client = client;
    request_t *rq = (request_t *)malloc(sizeof(request_t) + head.body_size);
    if (!rq) {
        LOG_ERR("malloc requst failed");
        return NULL;
    }

    memcpy(rq, &head, sizeof(head));

    p = &rq->body;
    left = rq->body_size;
    while (left) {
        n = read(client->fd, p, left);
        if (n == -1) {
            if (errno == EINTR)
                continue;

            LOG_ERR("read error : %d", n);
            return NULL;
        } else if (n == 0) {
            LOG_ERR("client close");
            close(client->fd);
            return NULL;

        }
            
        left -= n;
       p += n;
    }
    LOG_DEBUG("%s() end", __func__);
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
        LOG_DEBUG("error type %d", rq->type);
        return 1;
    }

    return 0;
}


int send_response(client_t *client, int ret, int type, char *msg) {

    LOG_DEBUG("send_response, client->fd = %d, ret = %d, type = %d, msg = %s", client->fd,  ret, type, msg);
    int msg_len = 0;
    if (msg) {
        msg_len = strlen(msg);
    }

    int resp_len = sizeof(response_t) + msg_len;
    response_t *resp = (response_t *)malloc(resp_len);
    if (!resp) {
        LOG_ERR("malloc resonse failed]");
        return NULL;
    }

    resp->ret = ret;
    resp->type = type;
    resp->msg_len = msg_len;
    if (resp->msg_len) {
        strcpy(resp->msg, msg);
    }

    int left = resp_len;
    char *p = resp;
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
    LOG_DEBUG("login request: %s,%s", rq->from, rq->to);
    rq->client->last_active_time = time(NULL);    

    user_t *user = search_user(rq->from);
    if (!user) {
        send_response(rq->client, RET_FAIL, RQ_LOGIN_TYPE, "user doesn't exist");
    } else {
        if (!strcmp(user->passwd, rq->to)) {
            int another_login = 0;
            client_t *client;
            for_each_client(&clients_info, client) {
                if (client->user == user) {
                    another_login = 1;
                }
            }

            if (another_login) {
                send_response(rq->client, RET_FAIL, RQ_LOGIN_TYPE, "user has already loin");

            } else {
                send_response(rq->client, RET_SUCCESS, RQ_LOGIN_TYPE, "login success");
                //login success
                rq->client->user = user;
                
                //inform other acitive users
                char buf[1024];
                sprintf(buf, "[%s has login!]", user->name);
                client_t *client;
                for_each_client(&clients_info, client) {
                    if (client->user && client->user != user) {
                        send_response(client, RET_SUCCESS, RQ_UNREQUESTED, buf);
                    }
                }
            }
        }
        else 
            send_response(rq->client, RET_FAIL, RQ_LOGIN_TYPE, "passwd error");
    }
}

void do_register_request(request_t *rq) {
    LOG_DEBUG("register request: %s,%s", rq->from, rq->to);
    rq->client->last_active_time = time(NULL);    

    user_t *user = search_user(rq->from);
    if (user) {
        send_response(rq->client, RET_FAIL, RQ_LOGIN_TYPE, "user name has already registered!");
    } else {
        user_t *user = (user_t *)malloc(sizeof(user_t));
        if (!user) {
            LOG_ERR("malloc user failed");
            return;
        }
        strcpy(user->name, rq->from);
        strcpy(user->passwd, rq->to);
        add_save_user(user);

        rq->client->user = user;

        send_response(rq->client, RET_SUCCESS, RQ_REGISTER_TYPE, "register success!");

        //inform other acitive users
        char buf[1024];
        sprintf(buf, "[%s has login!]", user->name);
        client_t *client;
        for_each_client(&clients_info, client) {
            if (client->user && client->user != user) {
                send_response(client, RET_SUCCESS, RQ_UNREQUESTED, buf);
            }
        }
    }
}


void do_show_active_users_request(request_t *rq) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    char *p = buf;
    rq->client->last_active_time = time(NULL);    

    char *start = "[active users: ";
    char *end = "]";
    int n = sprintf(p, "%s", start);
    p += (n-1);

    client_t *client;
    for_each_client(&clients_info, client) {
        if (client->user) {
            LOG_DEBUG("active user: %s", client->user->name);
            sprintf(p, "%s ", client->user->name);
            p += strlen(client->user->name) + 1;
        }
    }

    n = sprintf(p, "%s", end);
    send_response(rq->client, RET_SUCCESS, RQ_SHOW_ACTIVE_USERS_TYPE, buf);
}


void do_snd_msg_request(request_t *rq) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    char *p = buf;

    LOG_DEBUG("%s() start", __func__);
    rq->client->last_active_time = time(NULL);    
    if (!rq->body_size) {
        send_response(rq->client, RET_FAIL, RQ_SND_MSG_TYPE, "null msg");
    } else {
        int find_to_user = 0; 
        client_t *client;
        for_each_client(&clients_info, client) {
            if (client->user) {
                if (!strcmp(client->user->name, rq->to)) {
                    find_to_user = 1;
                    break;
                }
            }
        }

        sprintf(p, "[%s send to %s]: %s", rq->from, rq->to, rq->body);
        if (find_to_user) {
            send_response(client, RET_SUCCESS, RQ_UNREQUESTED, buf);
            send_response(rq->client, RET_SUCCESS, RQ_SND_MSG_TYPE, NULL);
        } else {
            send_response(client, RET_FAIL, RQ_SND_MSG_TYPE, "user not exist or is offline");
        }
    }
    LOG_DEBUG("%s() end", __func__);
}

void do_snd_msg_all_request(request_t *rq) {
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    char *p = buf;

    LOG_DEBUG("do_snd_msg_all_request() start");
    rq->client->last_active_time = time(NULL);    
    if (!rq->body_size) {
        send_response(rq->client, RET_SUCCESS, RQ_SND_MSG_ALL_TYPE, buf);
    } else {
        sprintf(buf, "[%s snd to all ]: %s", rq->from, rq->body);

        client_t *client;
        for_each_client(&clients_info, client) {
            if (client->user) {
                if (!strcmp(client->user->name, rq->from)) {
                    send_response(client, RET_SUCCESS, RQ_SND_MSG_ALL_TYPE, buf); //snd to all also to himself
                } else {
                    send_response(client, RET_SUCCESS, RQ_UNREQUESTED, buf);
                }
            }
        }
    }
    LOG_DEBUG("do_snd_msg_all_request() end");
}

void do_heart_beat_request(request_t *rq) {
    rq->client->last_active_time = time(NULL);    
}

void do_logout_request(request_t *rq) {
    //inform other acitive users
    char buf[1024];
    sprintf(buf, "[%s has logout!]", rq->from);
    client_t *client;
    for_each_client(&clients_info, client) {
        if (client->user && strcmp(client->user->name, rq->from)) {
            send_response(client, RET_SUCCESS, RQ_UNREQUESTED, buf);
        }
    }
    rq->client->user = NULL; 
    send_response(rq->client, RET_SUCCESS, RQ_LOGOUT_TYPE, NULL);
}
