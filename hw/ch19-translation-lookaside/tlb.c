/*
 * tlb.c
 *
 * Roughly measures the average cost of accessing memory as a function of
 * how many distinct pages are touched, following the Saavedra-Barrera-style
 * method described in OSTEP Chapter 19 ("Paging: Faster Translations (TLBs)"),

 *
 * Usage: ./tlb <num_pages> <num_trials>
 *   num_pages  - number of distinct pages to touch each sweep
 *   num_trials - number of times to repeat the sweep (for timing accuracy)
 *
 * Output (stdout, space-separated, one line):
 *   <num_pages> <num_trials> <avg_ns_per_access>
 *
 */

/* Must be defined before any #include: this exposes POSIX declarations
 * (like _SC_PAGESIZE and CLOCK_MONOTONIC) that some libc/compiler
 * combinations otherwise hide under strict standards modes. */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    int  num_pages;
    long num_trials;
} args_t;

/* ---- time ---- */

/* Current time in seconds (as a double), from a monotonic clock.
 * clock_gettime(CLOCK_MONOTONIC) has much finer practical resolution than
 * gettimeofday() (see Question 1), which matters because each individual
 * access is only a few nanoseconds. */
static double now_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

/* ---- geometry: pagesize -> jump -> array size ---- */

static long get_pagesize(void) {
#ifdef _SC_PAGESIZE
    return sysconf(_SC_PAGESIZE);      /* actual system page size */
#else
    return 4096;                       /* fallback if not exposed */
#endif
}

static long ints_per_page(long pagesize) {
    return pagesize / sizeof(int);     /* one int touched per page */
}

static long total_ints(int num_pages, long jump) {
    return (long)num_pages * jump;     /* total ints spanned by the array */
}

/* ---- array setup ---- */

static volatile int *allocate_array(long array_ints) {
    /* `volatile` discourages the compiler from proving the sweep loop is
     * dead code and optimizing it away entirely (Question 5 covers this
     * issue -- and better fixes -- in more detail). */
    volatile int *a = calloc(array_ints, sizeof(int));
    if (a == NULL) {
        fprintf(stderr, "calloc failed for %ld ints\n", array_ints);
        exit(1);
    }
    return a;
}

/* One full sweep: touch exactly one int per page across the whole array. */
static void sweep_once(volatile int *a, long array_ints, long jump) {
    for (long i = 0; i < array_ints; i += jump) {
        a[i] += 1;
    }
}

/* Touch every page once *before* timing starts, so first-touch costs like
 * demand-zeroing page faults don't pollute the timed measurement
 * (Question 7 discusses this). Built directly on sweep_once(). */
static void warmup(volatile int *a, long array_ints, long jump) {
    sweep_once(a, array_ints, jump);
}

/* ---- timing ---- */

/* Time `num_trials` repeated sweeps as a single block -- because one access
 * is far too fast to time individually (see Question 1) -- and return the
 * total elapsed seconds. */
static double time_sweeps(volatile int *a, long array_ints, long jump, long num_trials) {
    double t0 = now_sec();
    for (long t = 0; t < num_trials; t++) {
        sweep_once(a, array_ints, jump);
    }
    return now_sec() - t0;
}

/* ---- result ---- */

static double ns_per_access(double total_seconds, long num_trials, int num_pages) {
    double total_accesses = (double)num_trials * (double)num_pages;
    return (total_seconds / total_accesses) * 1e9;
}

static void report(int num_pages, long num_trials, double ns) {
    printf("%d %ld %f\n", num_pages, num_trials, ns);
}

/* ---- orchestration ---- */

/* Builds the array, warms it up, times it, reports the result, and cleans
 * up. Notice ns_per_access() feeds straight into report(), and
 * time_sweeps() feeds straight into ns_per_access() -- each function's
 * output becomes the next function's input, instead of main() spelling out
 * every intermediate step itself. */
static void run(int num_pages, long num_trials) {
    long jump       = ints_per_page(get_pagesize());
    long array_ints = total_ints(num_pages, jump);
    volatile int *a = allocate_array(array_ints);

    warmup(a, array_ints, jump);

    report(num_pages, num_trials,
           ns_per_access(time_sweeps(a, array_ints, jump, num_trials),
                          num_trials, num_pages));

    free((void *)a);
}

static args_t parse_args(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <num_pages> <num_trials>\n", argv[0]);
        exit(1);
    }

    args_t args = { .num_pages = atoi(argv[1]), .num_trials = atol(argv[2]) };

    if (args.num_pages <= 0 || args.num_trials <= 0) {
        fprintf(stderr, "num_pages and num_trials must both be positive\n");
        exit(1);
    }

    return args;
}

int main(int argc, char *argv[]) {
    args_t args = parse_args(argc, argv);
    run(args.num_pages, args.num_trials);
    return 0;
}
