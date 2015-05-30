#ifndef _REQUEST_H_
#define _REQUEST_H_

#include "client.h"

typedef struct request {
    struct client *p;

    //read from socket
    int version;
    int id;
    int type;
    int body_size;
    char body[0];
} request_t;

struct request * read_request(struct client *p);
void do_request(struct request *rq);

#endif
