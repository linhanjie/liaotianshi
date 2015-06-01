#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>


#define RQ_REGISTER_TYPE 0
#define RQ_LOGIN_TYPE 1
#define RQ_LOGOUT_TYPE 2
#define RQ_SHOW_ACTIVE_USERS_TYPE 3
#define RQ_SND_MSG_TYPE 4
#define RQ_SND_MSG_ALL_TYPE 5
#define RQ_HEART_BEAT_TYPE 6
#define RQ_ERROR_TYPE 7

typedef struct request {
    int version;
    char from[20];
    char to[20];
    int type;
    int body_size;
    char body[0];
} request_t;


int login_sts = 0; //default no login

void process_login(char *buf) {
}

void process_logout(char *buf){
}

void process_register(char *buf) {
}

void process_show_active_users(char *buf) {
}

void process_snd_msg(char *buf){
}

void process_snd_msg_all(char *buf){
}

void process_heart_beat(char *buf) {
}


#define CMD_LOGIN "login:"   //login:name:passwd 
#define CMD_REGISTER "register:" //register:name:passwd 
#define CMD_SHOW_USERS "show:" //show:
#define CMD_SND_MSG "snd:"    //snd:name:msg
#define CMD_LOGOUT "logout:" //logout:

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


int main()
{
    int sockfd;
    int len;

    struct sockaddr_in address;
    int result;
    char ch = 'A';

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
            case RQ_LOGIN_TYPE: process_login(buf);break;
            case RQ_REGISTER_TYPE: process_register(buf);break;
            case RQ_SHOW_ACTIVE_USERS_TYPE: process_show_active_users(buf);break;
            case RQ_SND_MSG_TYPE:process_snd_msg(buf);break;
            case RQ_SND_MSG_ALL_TYPE:process_snd_msg_all(buf);break;
            case RQ_LOGOUT_TYPE:process_logout(buf);break;
            default:
                                printf("error type: %s\n", buf);
        
        }

    }

    close(sockfd);
    return 0;
}







