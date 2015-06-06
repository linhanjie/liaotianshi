#include <pthread.h>
#include  <sys/syscall.h>
#include <stdio.h>

#include "work_thread.h"
#include "request.h"
#include "log.h"

static pid_t gettid()
{
    return syscall(SYS_gettid);
}

static void work_thread(void *data) {
    struct message_queue *queue = (struct message_queue*)data;
    LOG_DEBUG("%s() start", __func__);

    struct request *rq;
    for (;;) {
        dequeue_message(queue, &rq);
        printf(">>> thread: %d dequeue client fd = %d\n", gettid(), rq->client->fd);
        do_request(rq);
    }

}



void work_threads_init(struct message_queue *queue) {
    pthread_t tid;
    int i,ret;
    for (i=0; i<10; i++) {
        ret=pthread_create(&tid,NULL, work_thread, queue);
        if(ret!=0)
        {
            printf("Create pthread error!\n");
            return;
        }
    }

}



