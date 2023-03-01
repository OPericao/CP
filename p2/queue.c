#include <stdlib.h>
#include <pthread.h>

// circular array
typedef struct _queue {
    int size;
    int used;
    int first;
    void **data;
    pthread_mutex_t *mutex;
    pthread_cond_t *full;
    pthread_cond_t *empty;
} _queue;

#include "queue.h"

queue q_create(int size) {
    queue q = malloc(sizeof(_queue));

    q->size  = size;
    q->used  = 0;
    q->first = 0;
    q->data  = malloc(size * sizeof(void *));
    q->mutex = malloc(sizeof(pthread_mutex_t));
    q->full  = malloc(sizeof(pthread_cond_t));
    q->empty  = malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(q->mutex,NULL);
    pthread_cond_init(q->full,NULL);
    pthread_cond_init(q->empty,NULL);

    return q;
}

int q_elements(queue q) {
    return q->used;
}

int q_insert(queue q, void *elem) {
    pthread_mutex_lock(q->mutex);

    if(q->size == q->used){
        pthread_cond_wait(q->full,q->mutex);
    }
    q->data[(q->first + q->used) % q->size] = elem;
    q->used++;
    if(q->used == 1){
        pthread_cond_broadcast(q->empty);
    }
    pthread_mutex_unlock(q->mutex);
    return 0;
}

void *q_remove(queue q) {
    void *res;

    pthread_mutex_lock(q->mutex);
    if(q->used == 0) return NULL;

    res = q->data[q->first];

    q->first = (q->first + 1) % q->size;
    q->used--;
    pthread_mutex_unlock(q->mutex);
    return res;
}

void q_destroy(queue q) {
    free(q->data);
    free(q->mutex);
    free(q);
}
