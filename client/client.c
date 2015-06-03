#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
 #include <semaphore.h>
#include <pthread.h>

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


response_t *receive_response() {
    static response_t head;
    response_t *resp = NULL;
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
        }

        left -= n;
        p += n;
    }
   // printf("head ret=%d, type=%d, msg_len=%d\n", head.ret, head.type, head.msg_len);

    if (head.msg_len) {
        if (head.msg_len > 1024)
            printf("warning: msg_len is %d big\n", head.msg_len);
    
    
        resp = (response_t *)malloc(sizeof(response_t) + head.msg_len);
        if (!resp) {
            printf("malloc response failed\n");
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

    } else
        return &head;    

    return resp;
}

int send_request(request_t *rq) {

    int size = sizeof(*rq) + rq->body_size;
    char *p = rq;
    int n;
    int left = size;
    while (left) {
        n = write(sockfd, p, left);
        if (n == -1) {
            if (errno == EINTR)
                continue;
            else {
                perror("send request failed\n");
                return 1;
            }
        }

        left -= n;
        p += n;
    }

    return 0;
}

request_t * new_request(char *from, char *to, int type, int body_size) {

    request_t *rq = (request_t *)malloc(sizeof(request_t) + body_size);
    if (!rq) {
        printf("malloc request failed\n");
        return rq;
    }

    if (from) 
        strcpy(rq->from, from);

    if (to)
        strcpy(rq->to, to);

    rq->type = type;

    rq->body_size = body_size;

    return rq;
}

int check_name_passwd(char *str) {
    int len = strlen(str);
    if (len < 3 || len > 19) {
        printf("%s lenght error, should 3 to 19\n");
        return 1;
    }

    int i;
    for (i=0; i<len; i++) 
    {
        if (str[i] > 128 || str[i] == ':') {
            printf("%s iliegal character\n", str);
            return 1;
        }
    }

    return 0;
}


void process_login(char *buf) {
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
    printf("name = %s\n", name);
    if (check_name_passwd(name))
        return;

    char *passwd = p;
    printf("passwd = %s\n", passwd);
    if (check_name_passwd(passwd))
        return;

    request_t *rq = new_request(name, passwd, RQ_LOGIN_TYPE, 0);

    if (send_request(rq))
        return;

    sem_wait(&sem_resp);

}


void process_register(char *buf) {
}

void process_show_active_users(char *buf) {
    char *p = buf;
    //skip login:
    p += strlen(CMD_SHOW_USERS);

    request_t *rq = new_request(login_name, NULL, RQ_SHOW_ACTIVE_USERS_TYPE, 0);

    if (send_request(rq))
        return;

}

void process_logout(char *buf){
}


void process_snd_msg(char *buf){
}

void process_snd_msg_all(char *buf){
}

void process_heart_beat(char *buf) {
}


int get_cmd(char *buf) {
    int type = RQ_ERROR_TYPE;

    if (!login_sts) {
        printf("[== please first login(login:name:passwd) or register a new account(register:name:passwd) ==]\n");
    } else {
        printf("[== default snd msg to all! other cmds: show active users(show:), snd msg to person(snd:user:), logout(quit:) ==]\n");
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
            type = RQ_SND_MSG_TYPE; }
        else if (!strncmp(buf, CMD_LOGOUT, strlen(CMD_LOGOUT))){
            type = RQ_LOGOUT_TYPE;
        } else {
            type = RQ_SND_MSG_ALL_TYPE;
        }
    }

    return type;
}

void process_login_resp(response_t *resp) {
    if (resp->ret) {//error
        printf("login failed:");
        if (resp->msg_len)
            printf("%s\n", resp->msg);
    } else {
        if (resp->msg_len)
            printf("%s\n", resp->msg);

        //login success
        login_sts = 1;
    }
}

void process_register_resp(response_t *resp) {
}

void process_show_active_users_resp(response_t *resp) {
    if (resp->ret) {//error
        printf("%s : ", __func__);
        if (resp->msg_len)
            printf("%s\n", resp->msg);
    } else {
        if (resp->msg_len)
            printf("%s\n", resp->msg);
    }
}

void process_snd_msg_resp(response_t *resp) {
}

void process_snd_msg_all_resp(response_t *resp) {
}

void process_logout_resp(response_t *resp) {
}

void * receive_thread(void * data) {
    fd_set fdsr;
    int max_fd = sockfd + 1;
    response_t *resp;

    printf("receive_thread() runing...\n");
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
            printf("timeout\n");
            continue;
        }

        if (FD_ISSET(sockfd, &fdsr)) {
            resp = receive_response();

            if (!resp) {
                printf("receive_response failed\n");
                break;
            }

            if (resp->type == RQ_UNREQUESTED) {
                if (resp->msg_len);
                printf("%s\n", resp->msg);
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
                    printf("error type: %d\n", resp->type);
                    resp_error = 1;
                }

                if (resp_error)
                    break;
                else
                    sem_post(&sem_resp);
            }
        }

    }
    printf("receive_thread() end....\n");
}
int main(int argc, char **argv)
{
    int len;

    struct sockaddr_in address;
    int result;
    char ch = 'A';

    sem_init(&sem_resp, 0, 0);


    pthread_t tid;
    pthread_create(&tid,NULL, receive_thread,  NULL);


    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(2323);
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr*)&address, len);

    if (result == -1) {
        perror("oops: client1");
        return 1;
    }


    printf("--------------------------------------\n");
    printf("------ connect server success --------\n");
    printf("--------------------------------------\n\n");


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
            printf("error type: %s\n", buf);

        }

    }

    close(sockfd);
    return 0;
}

