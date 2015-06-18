#include <time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <pthread.h>
#include "../log.h"

#define RQ_REGISTER_TYPE 0
#define RQ_LOGIN_TYPE 1
#define RQ_LOGOUT_TYPE 2
#define RQ_SHOW_ACTIVE_USERS_TYPE 3
#define RQ_SND_MSG_TYPE 4
#define RQ_SND_MSG_ALL_TYPE 5
#define RQ_HEART_BEAT_TYPE 6
#define RQ_ERROR_TYPE 7
#define RQ_UNREQUESTED 100


#define CMD_LOGIN "login:"   //login:name:passwd 
#define CMD_REGISTER "register:" //register:name:passwd 
#define CMD_SHOW_USERS "show:" //show:
#define CMD_SND_MSG "snd:"    //snd:name:msg
#define CMD_SNDALL_MSG "sndall:"    //sndall:msg
#define CMD_LOGOUT "logout:" //logout:


typedef struct request {
    int version;
    char from[20];
    char to[20];
    int type;
    int body_size;
    char body[0];
} request_t;


typedef struct response {
    int ret;
    int type;
    int msg_len;
    char msg[0];
} response_t;

int login_sts = 0; //default no login
char login_name[20];
int snd_to_mode = 0; //snd to person private mode
char snd_to_name[20];
int sockfd;
sem_t sem_resp;
pthread_mutex_t rq_lock;

response_t *receive_response() {
    LOG_DEBUG("receive_response() start");
    static response_t head;
    response_t *resp = &head;
    int n;
    int left;
    left = sizeof(head);
    char *p = &head;

    while (left) {
        n = read(sockfd, p, left);

        if (n == -1) {
            if (errno == EINTR) 
                continue;
            else {
                perror("read socket failed");
                return NULL;
            }
        } else if (n == 0) {
            LOG_DEBUG("server close()");
            return NULL;
        }

        left -= n;
        p += n;
    }
   // LOG_DEBUG("head ret=%d, type=%d, msg_len=%d", head.ret, head.type, head.msg_len);

    if (head.msg_len) {
        LOG_DEBUG("receive_response() body %d start", head.msg_len);
        if (head.msg_len > 1024)
            LOG_DEBUG("warning: msg_len is %d big", head.msg_len);
    
    
        resp = (response_t *)malloc(sizeof(response_t) + head.msg_len);
        if (!resp) {
            LOG_DEBUG("malloc response failed");
            return NULL;
        }

        memcpy(resp, &head, sizeof(head));
        
        left = resp->msg_len;
        p = &resp->msg;
        while (left) 
        {
            n = read(sockfd, p, left);
            if (n == -1) {
                if (errno == EINTR)
                    continue;
                else {
                    perror("read msg failed");
                    free(resp);
                    return NULL;
                }
            }

            left -= n;
            p += n;
        }

    } 
    
    LOG_DEBUG("receive_response() end");
    return resp;
}

int send_request(request_t *rq) {
    LOG_DEBUG("%s() start", __func__);
    pthread_mutex_lock(&rq_lock);

    int size = sizeof(*rq) + rq->body_size;
    char *p = rq;
    int n;
    int left = size;
    LOG_DEBUG("%s() send size = %d", __func__, left);
    while (left) {
        n = write(sockfd, p, left);
        if (n == -1) {
            if (errno == EINTR)
                continue;
            else {
                perror("send request failed");
                return 1;
            }
        }

        left -= n;
        p += n;
    }
    pthread_mutex_unlock(&rq_lock);
    LOG_DEBUG("%s() end", __func__);
    return 0;
}

request_t * new_request(char *from, char *to, int type, char *body_msg) {

    int body_size = 0;
    if (body_msg)
        body_size = strlen(body_msg);

    request_t *rq = (request_t *)malloc(sizeof(request_t) + body_size);
    if (!rq) {
        LOG_DEBUG("malloc request failed");
        return rq;
    }

    if (from) 
        strcpy(rq->from, from);

    if (to)
        strcpy(rq->to, to);

    rq->type = type;

    rq->body_size = body_size;

    if (rq->body_size) {
        strcpy(rq->body, body_msg);
    }
    return rq;
}

int check_name_passwd(char *str) {
    int len = strlen(str);
    if (len < 3 || len > 19) {
        LOG_DEBUG("%s lenght error, should 3 to 19");
        return 1;
    }

    int i;
    for (i=0; i<len; i++) 
    {
        if (str[i] > 128 || str[i] == ':') {
            LOG_DEBUG("%s iliegal character", str);
            return 1;
        }
    }

    return 0;
}


void process_login(char *buf) {
    LOG_DEBUG("%s() start", __func__);
    char *p = buf;

    //skip login:
    p += strlen(CMD_LOGIN);

    char *name = p;
    while (*p) {
        if (*p == ':')
            break;
        p++;
    }

    *p = 0;
    p++;
    LOG_DEBUG("name = %s", name);
    if (check_name_passwd(name))
        return;
    strcpy(login_name, name);

    char *passwd = p;
    LOG_DEBUG("passwd = %s", passwd);
    if (check_name_passwd(passwd))
        return;

    request_t *rq = new_request(name, passwd, RQ_LOGIN_TYPE, NULL);

    if (send_request(rq))
        return;
    
    LOG_DEBUG("%s() wait response...", __func__);

    sem_wait(&sem_resp);
    LOG_DEBUG("%s() end", __func__);
}


void process_register(char *buf) {
    LOG_DEBUG("%s() start", __func__);
    char *p = buf;

    //skip login:
    p += strlen(CMD_REGISTER);

    char *name = p;
    while (*p) {
        if (*p == ':')
            break;
        p++;
    }

    *p = 0;
    p++;
    LOG_DEBUG("name = %s", name);
    if (check_name_passwd(name))
        return;
    strcpy(login_name, name);

    char *passwd = p;
    LOG_DEBUG("passwd = %s", passwd);
    if (check_name_passwd(passwd))
        return;

    request_t *rq = new_request(name, passwd, RQ_REGISTER_TYPE, NULL);

    if (send_request(rq))
        return;
    
    LOG_DEBUG("%s() wait response...", __func__);

    sem_wait(&sem_resp);
    LOG_DEBUG("%s() end", __func__);
}

void process_show_active_users(char *buf) {

    request_t *rq = new_request(login_name, NULL, RQ_SHOW_ACTIVE_USERS_TYPE, NULL);
    if (send_request(rq))
        return;

    sem_wait(&sem_resp);
}

void process_logout(char *buf){
    request_t *rq = new_request(login_name, NULL, RQ_LOGOUT_TYPE, NULL);

    if (send_request(rq))
        return;

    sem_wait(&sem_resp);
}


void process_snd_msg(char *buf) {
    char *p = buf;
    if (!strncmp(buf, CMD_SND_MSG, strlen(CMD_SND_MSG))) 
    {
        p += strlen(CMD_SND_MSG);

        char *name = p;
        while (*p) {
            if (*p == ':')
                break;
            p++;
        }

        *p = 0;
        p++;
        LOG_DEBUG("name = %s", name);
        if (check_name_passwd(name))
            return;
        strcpy(snd_to_name, name);
    }

    request_t *rq = new_request(login_name, snd_to_name, RQ_SND_MSG_TYPE, p);

    if (send_request(rq))
        return;

}

void process_snd_msg_all(char *buf) {

    char *p = buf;
    if (!strncmp(buf, CMD_SNDALL_MSG, strlen(CMD_SNDALL_MSG))) 
    {
        p += strlen(CMD_SNDALL_MSG);
    }

    request_t *rq = new_request(login_name, NULL, RQ_SND_MSG_ALL_TYPE, buf);

    if (send_request(rq))
        return;

    sem_wait(&sem_resp);
}

void process_heart_beat(char *buf) {
}


int get_cmd(char *buf) {
    int type = RQ_ERROR_TYPE;

    if (!login_sts) {
        LOG_DEBUG("[== please first login(login:name:passwd) or register a new account(register:name:passwd) ==]");
    } else if (snd_to_mode) {
        LOG_DEBUG("[== default send to %s,  show active users(show:), snd msg to person(snd:user:), snd msg to all(sndall:), logout(quit:) ==]", snd_to_name);
    } else {
        LOG_DEBUG("[== default send to all. show active users(show:), snd msg to person(snd:user:), snd msg to all(sndall:), logout(quit:) ==]");
    }


    gets(buf);

    if (!login_sts) {
        if (!strncmp(buf, CMD_LOGIN, strlen(CMD_LOGIN))) {
            type = RQ_LOGIN_TYPE;
        } else if (!strncmp(buf, CMD_REGISTER, strlen(CMD_REGISTER))) {
            type=RQ_REGISTER_TYPE;
        }
    } else {
        if (!strncmp(buf, CMD_SHOW_USERS, strlen(CMD_SHOW_USERS))) {
            type = RQ_SHOW_ACTIVE_USERS_TYPE;
        } else if (!strncmp(buf, CMD_SND_MSG, strlen(CMD_SND_MSG))) {
            type = RQ_SND_MSG_TYPE; 
        } else if (!strncmp(buf, CMD_SNDALL_MSG, strlen(CMD_SNDALL_MSG))) {
            type = RQ_SND_MSG_ALL_TYPE;
        }
        else if (!strncmp(buf, CMD_LOGOUT, strlen(CMD_LOGOUT))){
            type = RQ_LOGOUT_TYPE;
        } else {
            if (snd_to_mode) {
                type = RQ_SND_MSG_TYPE; 
            } else {
                type = RQ_SND_MSG_ALL_TYPE;
            }
        }
    }

    return type;
}

void process_login_resp(response_t *resp) {
    if (resp->ret) {//error
        LOG_DEBUG("login failed:");
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);
    } else {
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);

        //login success
        login_sts = 1;
    }
}

void process_register_resp(response_t *resp) {
    if (resp->ret) {//error
        LOG_DEBUG("[register failed] : ");
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);
    } else {
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);

        //login success
        login_sts = 1;
    }
}

void process_show_active_users_resp(response_t *resp) {
    if (resp->ret) {//error
        LOG_DEBUG("%s : ", __func__);
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);
    } else {
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);
    }
}

void process_snd_msg_resp(response_t *resp) {
    LOG_DEBUG("%s() start", __func__);
    if (resp->ret) {//error
        LOG_DEBUG("%s : ", __func__);
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);

        snd_to_mode = 0;
    } else {
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);

        snd_to_mode = 1;
    }

}

void process_snd_msg_all_resp(response_t *resp) {
    LOG_DEBUG("%s() start", __func__);
    if (resp->ret) {//error
        LOG_DEBUG("%s : ", __func__);
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);
    } else {
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);

        snd_to_mode = 0;
    }
}

void process_logout_resp(response_t *resp) {
    LOG_DEBUG("%s() start", __func__);
    if (resp->ret) {//error
        LOG_DEBUG("%s : ", __func__);
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);
    } else {
        if (resp->msg_len)
            LOG_DEBUG("%s", resp->msg);

        login_sts = 0;
    }
    LOG_DEBUG("%s() end", __func__);
}

void * receive_thread(void * data) {
    fd_set fdsr;
    int max_fd = sockfd + 1;
    response_t *resp;

    LOG_DEBUG("receive_thread() runing...");
    while (1) {
        // initialize file descriptor set
        FD_ZERO(&fdsr);
        FD_SET(sockfd, &fdsr);

        int ret = select(max_fd + 1, &fdsr, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR)
                continue;

            perror("select");
            break;
        } else if (ret == 0) {
            LOG_DEBUG("timeout");
            continue;
        }

        if (FD_ISSET(sockfd, &fdsr)) {
            resp = receive_response();

            if (!resp) {
                LOG_DEBUG("receive_response failed");
                exit(1);
                break;
            } 

            if (resp->type == RQ_UNREQUESTED) {
                if (resp->msg_len);
                LOG_DEBUG("%s", resp->msg);
            } else {
                int resp_error = 0;
                switch(resp->type) {
                case RQ_LOGIN_TYPE: 
                    process_login_resp(resp);
                    break;
                case RQ_REGISTER_TYPE: 
                    process_register_resp(resp);
                    break;
                case RQ_SHOW_ACTIVE_USERS_TYPE: 
                    process_show_active_users_resp(resp);
                    break;
                case RQ_SND_MSG_TYPE:
                    process_snd_msg_resp(resp);
                    break;
                case RQ_SND_MSG_ALL_TYPE:
                    process_snd_msg_all_resp(resp);
                    break;
                case RQ_LOGOUT_TYPE:
                    process_logout_resp(resp);
                    break;
                default:
                    LOG_DEBUG("error type: %d", resp->type);
                    resp_error = 1;
                }

                if (resp_error)
                    break;
                else
                    sem_post(&sem_resp);
            }
        }

    }
    LOG_DEBUG("receive_thread() end....");
}


void * heart_beat_thread(void * data) {
    return NULL;
    while (1)
    {
        struct timeval tv = {1, 0}; /* 5 seconds*/

        if (-1 == select(0, NULL, NULL, NULL, &tv))
        {
            if (EINTR == errno)
            {
                continue; /* this means the process received a signal during waiting, just start over waiting. However this could lead to wait cycles >20ms and <40ms. */
            }

            perror("select()");
            exit(1);
        }

        /* do something every 5s */
        if (login_sts) {
            LOG_DEBUG("heart beat");
            request_t *rq = new_request(login_name, NULL, RQ_HEART_BEAT_TYPE, NULL);
            send_request(rq);
        }
    }

}

char *ip;
unsigned ip_addr;

int main(int argc, char **argv)
{


   ip = argv[1];
    int port = atoi(argv[2]);
    int len;

    struct sockaddr_in address;
    int result;
    char ch = 'A';

    sem_init(&sem_resp, 0, 0);
    pthread_mutex_init(&rq_lock, NULL);

    pthread_t tid;
    pthread_create(&tid,NULL, receive_thread,  NULL);

    pthread_create(&tid,NULL, heart_beat_thread,  NULL);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    ip_addr = inet_addr(ip);
    address.sin_port = htons(port);
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr*)&address, len);

    if (result == -1) {
        perror("oops: client1");
        return 1;
    }

    LOG_DEBUG("--------------------------------------");
    LOG_DEBUG("------ connect server success --------");
    LOG_DEBUG("--------------------------------------");


    char buf[1024];
    while (1) {
        int cmd = get_cmd(buf);

        switch(cmd) {
        case RQ_LOGIN_TYPE: 
            process_login(buf);
            break;
        case RQ_REGISTER_TYPE: 
            process_register(buf);
            break;
        case RQ_SHOW_ACTIVE_USERS_TYPE: 
            process_show_active_users(buf);
            break;
        case RQ_SND_MSG_TYPE:
            process_snd_msg(buf);
            break;
        case RQ_SND_MSG_ALL_TYPE:
            process_snd_msg_all(buf);
            break;
        case RQ_LOGOUT_TYPE:
            process_logout(buf);
            break;
        default:
            LOG_DEBUG("error type: %s", buf);

        }

    }

    close(sockfd);
    return 0;
}

