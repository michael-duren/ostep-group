#define _GNU_SOURCE // need this for sched.h to actually contain the gnu extension sched_getcpu
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <sched.h>
#include "gettime.h"

#define CORES 16 // just hard coding this to my machine spec

typedef struct __actr_t {
  int value;
  pthread_mutex_t lock;
} actr_t;
typedef struct __counter_t {
  int value;
  actr_t ctrs[CORES];
  int thresh;
  pthread_mutex_t lock;
} counter_t;

typedef struct __worker_args_t {
  int num_updates;
  counter_t counter;
} worker_args_t;

void init(counter_t *c, int thresh) {
  c->value = 0;
  c->thresh = thresh;
  pthread_mutex_init(&c->lock, NULL);
  for (int i = 0; i < CORES; i++) {
    pthread_mutex_init(&c->ctrs[i].lock, NULL);
  }
}

void increment(counter_t *c) {
  // this could technically change between this line and the increment. 
  // doing this inside the lock wouldn't even change that 
  // this is just a race condition with performance cost though, not a logical error
  int cpu = sched_getcpu();
  pthread_mutex_lock(&c->ctrs[cpu].lock);
  c->ctrs[cpu].value++;
  if (c->ctrs[cpu].value >= c->thresh) {
    pthread_mutex_lock(&c->lock);
    c->value += c->ctrs[cpu].value;
    pthread_mutex_unlock(&c->lock);
    c->ctrs[cpu].value = 0;
  }
  pthread_mutex_unlock(&c->ctrs[cpu].lock);

}

// skipping decrement and get

void *work(void *args) {
  worker_args_t *wa = (worker_args_t*)args;
  for (int i = 0; i < wa->num_updates; i++) {
    increment(&wa->counter);
  }
  return 0;
}

int main(int count, char **args) {
  long starttime = get_mono_time();

  int num_threads = atoi(args[1]);
  int num_updates = atoi(args[2]);
  int approx_thresh = atoi(args[3]);

  worker_args_t worker_args;
  worker_args.num_updates = num_updates;
  init(&worker_args.counter, approx_thresh);

  pthread_t threads[num_threads];
  for (int i = 0; i < num_threads; i++) {
    pthread_create(&threads[i], NULL, work, (void *)&worker_args);
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  long endtime = get_mono_time();
  long delta = endtime - starttime;

  printf("%i Threads\n%i Updates\n%i Final Count\n%ld ns\n", num_threads, num_updates, worker_args.counter.value, delta);
}
