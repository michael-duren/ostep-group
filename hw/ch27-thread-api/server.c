#include <bits/time.h>
#include <bsd/err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_RESPONSES 100
#define STACK_SIZE 0x100000

typedef struct {
	long long status;
	char *body;
} res_t;

static unsigned int counter = 0;
pthread_mutex_t m;

static char *read_path(const char *path) {
	puts("reading file");
	return NULL;
}

static void *worker(void *arg) {
	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);
	pthread_t tid = pthread_self();

	long long id = (long long)arg;
	// pretend to do work
	for (int i = 0; i < 10000; i++) {
		if (i == 0) {
			printf("doing some work with id: %lld\n", id);
		}
	}

	// increment global counter
	pthread_mutex_lock(&m);
	counter++;
	pthread_mutex_unlock(&m);

	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	// printf("thread ID: %ld, ended at: %ld", tid, start.tv_sec);

	res_t *res = malloc(sizeof(res_t));
	res->body = "he fwiend";
	res->status = id;

	long int elsapsed_ms = (end.tv_sec - start.tv_sec) * 1000LL +
						   (end.tv_nsec - start.tv_nsec) / 1000000LL;
	printf("total milliseconds by thread: %ld worker: %ld. global counter "
		   "increased to: %d\n",
		   tid, elsapsed_ms, counter);
	return res;
}

struct thread_info {	 /* Used as argument to thread_start() */
	pthread_t thread_id; /* ID returned by pthread_create() */
	int thread_num;		 /* Application-defined thread # */
	char *argv_string;	 /* From command-line argument */
};

int main(int argc, char *argv[]) {
	pthread_t ids[MAX_RESPONSES] = {0};
	pthread_mutex_init(&m, NULL);

	for (long long i = 0; i < MAX_RESPONSES; i++) {
		pthread_t p;
		pthread_attr_t attr;
		int rc = pthread_attr_init(&attr);
		if (rc != 0) {
			errc(EXIT_FAILURE, rc, "pthread_attr_init");
		}
		rc = pthread_attr_setstacksize(&attr, STACK_SIZE);
		if (rc != 0) {
			errc(EXIT_FAILURE, rc, "pthread_attr_setstacksize");
		}
		rc = pthread_create(&p, &attr, &worker, (void *)i);
		if (rc != 0) {
			errc(EXIT_FAILURE, rc, "pthread_create");
		}
		ids[i] = p;
	}

	for (long long i = 0; i < MAX_RESPONSES; i++) {
		void *r;
		pthread_t curr = ids[i];
		int rc = pthread_join(curr, &r);
		if (rc != 0) {
			errc(EXIT_FAILURE, rc, "pthread_join");
		}
		res_t res = *(res_t *)r;
		printf("thread: %ld, responded with: %lld\n", curr, res.status);
		free(r);
	}

	return EXIT_SUCCESS;
}
