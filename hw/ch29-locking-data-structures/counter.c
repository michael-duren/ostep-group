#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include "gettime.h"
typedef struct __counter_t {
  int value;
  pthread_mutex_t lock;
} counter_t;

typedef struct __worker_args_t {
  int num_updates;
  counter_t counter;
} worker_args_t;

void init(counter_t *c) {
  c->value = 0;
  pthread_mutex_init(&c->lock, NULL);
}

void increment(counter_t *c) {
  pthread_mutex_lock(&c->lock);
  c->value++;
  pthread_mutex_unlock(&c->lock);
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

  worker_args_t worker_args;
  worker_args.num_updates = num_updates;
  init(&worker_args.counter);

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
