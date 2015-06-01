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

void do_request(struct request *rq) {
 //   printf("client %d request, version = %p, id = %d, type = %d, body_size = %d\n", rq->p->fd, rq->version, rq->id, rq->type, rq->body_size);
}

