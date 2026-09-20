
1. Basically use a timer and learn its smallest interval

Answer
- `timer.c` is copied from `man clock_gettime` example 
- It gives us nanoseconds using ts.tv_sec + ts.tv_nsec 
- I used gettime.h / gettime.c with get_mono_time() to grab monotonic clock timestamp in ns
- with a monotonic clock, an arbitrary point in the past is selected as the epoch for each process, but crucially, it remains the same across threads of the same process.

2. Build a simple concurrent counter and measure how long it takes to increment the counter many times as the number of threads increases. 
- How many CPUs are available on the system you are using? 
    - 16 (`$(nproc --all)`)
- Does this number impact your measurements?
    - yes

Answer
- `counter.c`
```
~/projects/ostep-group/hw/ch29-locking-data-structures week7*                                                                                                                      11:53:51
❯ ./counter 10 10
10 Threads
10 Updates
100 Final Count
431962 ns
~/projects/ostep-group/hw/ch29-locking-data-structures week7*                                                                                                                      11:53:52
❯ ./counter 1 100
1 Threads
100 Updates
100 Final Count
121158 ns
~/projects/ostep-group/hw/ch29-locking-data-structures week7*                                                                                                                      11:54:22
❮ ./counter 100 1
100 Threads
1 Updates
100 Final Count
3070553 ns
~/projects/ostep-group/hw/ch29-locking-data-structures week7*                                                                                                                      11:54:29
```

3. Same as #2 but with an approximate counter

Answer
- `acounter.c`
```
~/projects/ostep-group/hw/ch29-locking-data-structures week7*                                                                                                                      12:35:29
❯ ./acounter 10 1000 10
10 Threads
1000 Updates
9886 Final Count
427183 ns
~/projects/ostep-group/hw/ch29-locking-data-structures week7*                                                                                                                      12:35:41
❯ ./counter 10 1000
10 Threads
1000 Updates
10000 Final Count
525639 ns
```


