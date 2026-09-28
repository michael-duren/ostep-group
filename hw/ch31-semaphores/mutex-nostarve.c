#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include "common_threads.h"

#define MAX_THREADS 128

//
// Here, you have to write (almost) ALL the code. Oh no!
// How can you show that a thread does not starve
// when attempting to acquire this mutex you build?
//

typedef struct __ns_mutex_t {
    sem_t mutex;               // protects the ticket counters
    sem_t queue[MAX_THREADS];  // one "your turn" semaphore per ticket slot
    int next_ticket;
    int now_serving;
} ns_mutex_t;

void ns_mutex_init(ns_mutex_t *m) {
    sem_init(&m->mutex, 0, 1);
    for (int i = 0; i < MAX_THREADS; i++)
        sem_init(&m->queue[i], 0, 0);
    sem_post(&m->queue[0]);   // ticket 0 can go immediately
    m->next_ticket = 0;
    m->now_serving = 0;
}

void ns_mutex_acquire(ns_mutex_t *m) {
    sem_wait(&m->mutex);
    int my_ticket = m->next_ticket++;
    sem_post(&m->mutex);
    sem_wait(&m->queue[my_ticket % MAX_THREADS]);
}

void ns_mutex_release(ns_mutex_t *m) {
    sem_wait(&m->mutex);
    m->now_serving++;
    sem_post(&m->queue[m->now_serving % MAX_THREADS]);
    sem_post(&m->mutex);
}

int loops;
ns_mutex_t m;

void *worker(void *arg) {
    int id = (int)(long) arg;
    for (int i = 0; i < loops; i++) {
        ns_mutex_acquire(&m);
        printf("thread %d: got the mutex\n", id);
        usleep(50000);   // hold it briefly so ordering is actually visible
        ns_mutex_release(&m);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 3);
    int num_threads = atoi(argv[1]);
    loops = atoi(argv[2]);
    assert(num_threads <= MAX_THREADS);

    pthread_t p[num_threads];
    ns_mutex_init(&m);

    printf("parent: begin\n");
    for (int i = 0; i < num_threads; i++)
        Pthread_create(&p[i], NULL, worker, (void *)(long) i);
    for (int i = 0; i < num_threads; i++)
        Pthread_join(p[i], NULL);
    printf("parent: end\n");
    return 0;
}

