#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long get_mono_time() {
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    return (long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}
