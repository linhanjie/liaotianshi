#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "message_queue.h"


void message_queue_init(message_queue_t *queue) {
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->size = 0;
    queue->head = NULL;
}

int enqueue_message(message_queue_t *queue, int value) {
    pthread_mutex_lock(&queue->mutex);

    message_t *p = (message_t *)malloc(sizeof(message_t));
    if (!p) {
        printf("error, can't malloc new message\n");
        return 1;
    } else {
        p->next = NULL;
        p->prev = NULL;
        p->value = value;

        if (!queue->size) {
            queue->head = p;

        } else {
            queue->head->next = p;
            p->prev = queue->head;
        
            queue->head = p;
        }
        queue->size++;

        pthread_cond_signal(&queue->cond);

    }

    pthread_mutex_unlock(&queue->mutex);

    return 0;
}

int dequeue_message(message_queue_t *queue, int *value) {

    pthread_mutex_lock(&queue->mutex);

    if (!queue->size) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    } 
        
        if (!queue->size) {
            printf("error, queue size still= 0, can't dequeue_message\n");
            return 1;
        } else {
            message_t *p = queue->head;
            if (queue->size == 1) {
                queue->head = NULL;
            } else {
                queue->head = queue->head->prev;
                queue->head->next = NULL;
            }

            queue->size--;

            *value = p->value;
            free(p);
        }
    pthread_mutex_unlock(&queue->mutex);

    return 0;
}

