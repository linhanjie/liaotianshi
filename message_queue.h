#ifndef _MESSAGE_QUEUE_H_
#define _MESSAGE_QUEUE_H_


#include <stdio.h>
#include <pthread.h>

typedef struct message {
    struct message *next;
    struct message *prev;

    /* message stuff start */
    int value;
    /* message stuff end */
} message_t;


typedef struct message_queue {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int size;
    message_t *head;
} message_queue_t;


void message_queue_init(message_queue_t *queue);

int enqueue_message(message_queue_t *queue, int value);

int dequeue_message(message_queue_t *queue, int *value);

#endif
