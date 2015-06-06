#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "message_queue.h"
#include "log.h"

void message_queue_init(message_queue_t *queue) {
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->size = 0;
    queue->head = NULL;
}

int enqueue_message(message_queue_t *queue, void *data) {
    pthread_mutex_lock(&queue->mutex);

    LOG_DEBUG("enqueue_message: data = [%p]", data);

    message_t *p = (message_t *)malloc(sizeof(message_t));
    if (!p) {
        LOG_ERR("error, can't malloc new message");
        return 1;
    } else {
        p->next = NULL;
        p->prev = NULL;
        p->data = data;

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

int dequeue_message(message_queue_t *queue, void **pdata) {


    while (1) {
        pthread_mutex_lock(&queue->mutex);

        if (!queue->size) {
            pthread_cond_wait(&queue->cond, &queue->mutex);
        } 

        if (!queue->size) {
            LOG_ERR("error, queue size still= 0, can't dequeue_message");
            pthread_mutex_unlock(&queue->mutex);
            continue;
        } else {
            message_t *p = queue->head;
            if (queue->size == 1) {
                queue->head = NULL;
            } else {
                queue->head = queue->head->prev;
                queue->head->next = NULL;
            }

            queue->size--;

            *pdata = p->data;
            free(p);

            LOG_DEBUG("dequeue_message: data = [%p]", *pdata);
            pthread_mutex_unlock(&queue->mutex);
            break;
        }

    }
    return 0;
}

