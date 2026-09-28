# Homework (Code)
In this homework, we’ll use semaphores to solve some well-known
concurrency problems. Many of these are taken from Downey’s excellent
“Little Book of Semaphores”3, which does a good job of pulling together
a number of classic problems as well as introducing a few new variants;
interested readers should check out the Little Book for more fun.
Each of the following questions provides a code skeleton; your job is
to fill in the code to make it work given semaphores. On Linux, you
will be using native semaphores; on a Mac (where there is no semaphore
support), you’ll have to first build an implementation (using locks and
condition variables, as described in the chapter). Good luck!

# Questions
1. The first problem is just to implement and test a solution to the fork/join
problem, as described in the text. Even though this solution is described in
the text, the act of typing it in on your own is worthwhile; even Bach would
rewrite Vivaldi, allowing one soon-to-be master to learn from an existing
one. See fork-join.c for details. Add the call sleep(1) to the child to
ensure it is working.
        filled in the 3 lines in fork-join.c
        sem_post(&s);
        sem_init(&s, 0, 0);
        sem_wait(&s);
        `gcc -o fork-join fork-join.c -lpthread`
        `./fork-join`
        works after adding sleep(1)

2. Let’s now generalize this a bit by investigating the rendezvous problem.
The problem is as follows: you have two threads, each of which are about
to enter the rendezvous point in the code. Neither should exit this part of
the code before the other enters it. Consider using two semaphores for this
task, and see rendezvous.c for details.
    added sem_post(); and sem_wait(); to both children then initialized them in main 
    sem_init(&s1, 0, 0);
    sem_init(&s2, 0, 0);
    `gcc -o rendezvous rendezvous.c -lpthread`
    `./rendezvous`
    works after adding sleep(1) in child 1

3. Now go one step further by implementing a general solution to barrier synchronization. Assume there are two points in a sequential piece of code,
called P1 and P2. Putting a barrier between P1 and P2 guarantees that all
threads will execute P1 before any one thread executes P2. Your task: write
the code to implement a barrier() function that can be used in this manner. It is safe to assume you know N (the total number of threads in the
running program) and that all N threads will try to enter the barrier. Again,
you should likely use two semaphores to achieve the solution, and some
other integers to count things. See barrier.c for details.
    two semaphores: mutex (protects a shared counter) and barrier (the actual gate)
    every thread locks mutex, increments count, unlocks mutex, then waits on the gate
    whichever thread happens to push count up to N (the last to arrive) is
    responsible for releasing everyone: posts to the gate N times, resets count to 0
    Result: all 5 "before" printed before any "after"
    `gcc -o barrier barrier.c -lpthread`
    `./barrier 5`

4. Now let’s solve the reader-writer problem, also as described in the text. In
this first take, don’t worry about starvation. See the code in reader-writer.c
for details. Add sleep() calls to your code to demonstrate it works as you
expect. Can you show the existence of the starvation problem?
    readers share access, first reader in grabs the writelock, last reader out releases it, writer just
    grabs writelock directly
    `gcc -o reader-writer reader-writer.c -lpthread`
    `./reader-writer 1 1 5`
    reader's 5 loops finished before writer started, correct output but proves nothing (only 1
    reader, no contention possible)
    Starvation demo: added usleep(100000) inside reader()'s critical
    section, ran 
    `./reader-writer 20 1 5`
    Result: all 100 reads (20 readers x 5 loops) completed before the
    writer got a single turn, readers count never dropped to 0 since
    a new reader always started its next loop before the last one
    finished, so the writer's writelock request just sat there the
    whole time. Confirms the exact starvation risk called out in the
    chapter: this lock design has no mechanism to make new readers
    wait once a writer is queued up.

5. Let’s look at the reader-writer problem again, but this time, worry about
starvation. How can you ensure that all readers and writers eventually
make progress? See reader-writer-nostarve.c for details.
    added a turnstile semaphore: readers must pass through it before checking
    the readers count; a writer grabs turnstile first, blocking all new
    readers from entering while it waits for in-progress readers to finish
    `gcc -o reader-writer-nostarve reader-writer-nostarve.c -lpthread`
    `./reader-writer-nostarve 20 1 5` with usleep in reader()
    Result: only 20 reads happened before the writer got in (vs all 100 in
    Q4), writer ran its full 5 loops, then remaining 80 reads finished after
    Confirms the fix: the writer no longer waits for every reader to finish,
    just the ones already in flight when it showed up
    Side note: the writer ran all 5 of its iterations back-to-back once let
    in, since it wins the race to reopen/reclose the turnstile faster than
    a sleeping reader can wake up. Fixes writer starvation, doesn't
    guarantee perfectly even alternation.

6. Use semaphores to build a no-starve mutex, in which any thread that tries to
acquire the mutex will eventually obtain it. See the code in mutex-nostarve.c
for more information.
Clean, perfect round-robin: 0,1,2,3,4 repeated exactly 3 times (5 threads x 3 loops = 15 lines total), no thread ever gets skipped or repeats out of turn. That's about as clear a demonstration of FIFO fairness as you could ask for.
    each thread grabs a numbered ticket under a small internal mutex,
    then waits on its own dedicated semaphore slot; release always wakes
    whoever's ticket is next, not an arbitrary sleeper
    `gcc -o mutex-nostarve mutex-nostarve.c -lpthread`
    `./mutex-nostarve 5 3` (5 threads, 3 loops each)
    Result: threads acquired in strict order (0,1,2,3,4) repeated 3 times,
    exactly as expected. No thread skipped or got extra turns.
    Confirms: worst-case wait for any thread is bounded by however many
    threads are ahead of it in line, never indefinite, actual proof of
    no-starvation rather than just "seemed fine when I ran it"
    
7. Liked these problems? See Downey’s free text for more just like them. And
don’t forget, have fun! But, you always do when you write code, no?
    skip


Highlight: Q4 showed a naive reader-writer lock can starve a writer completely (100 reads before it got one turn), Q5's turnstile fix cut that to 20, and Q6's ticket-based mutex proved perfect FIFO fairness, together showing that mutual exclusion and fairness are separate guarantees, semaphores get you the first for free but you have to design for the second.