# CH27

## About

Unfortunately I don't get to write C, it looks like I'm just testing helgrind.

## Questions

1. Running

```bash
valgrind --tool=helgrind ./main-race
# output:
# gives the moment a new thread is created with an announcement
# with the line number in the c code, and when there is a data race
# gives context and line numbers with calling thread number the memory
# address and variable name of the data that is being updated
==2082719== This conflicts with a previous write of size 4 by thread #2
==2082719== Locks held: none
==2082719==    at 0x400117A: worker (main-race.c:6)
==2082719==    by 0x489147B: mythread_wrapper (hg_intercepts.c:410)
==2082719==    by 0x49390A1: start_thread (pthread_create.c:454)
==2082719==    by 0x49C1633: clone (clone.S:100)
==2082719==  Address 0x4004034 is 0 bytes inside data symbol "balance"
```

2. Commenting out a problamtic line like:

```c
	pthread_t p;
	Pthread_create(&p, NULL, worker, NULL);
	// balance++; // unprotected access
	Pthread_join(p, NULL);
```

Not suprisingly causes helgrind to report no issues.

3. My thought is: worker 1 will lock mutex 1 and then
   attempt to lock mutex 2 but it is already locked by worker 2.
   Thus each worker is waiting for the other to unlock someting and
   neither gets to the unlock portion.

   At least this is conceptially what I think the problem is, if I just
   run the program there seems to be no issue. It runs and returns exit code 0.

```c
void *worker(void *arg) {
	if ((long long)arg == 0) {
		Pthread_mutex_lock(&m1);
		Pthread_mutex_lock(&m2);
	} else {
		Pthread_mutex_lock(&m2);
		Pthread_mutex_lock(&m1);
	}
	Pthread_mutex_unlock(&m1);
	Pthread_mutex_unlock(&m2);
	return NULL;
}
```

4. Helgrind warns of possible deadlocks. Because the order of locking
   the mutex is 1 -> 2 by thread 2, an order or expected behavior of mutex
   is established. When thread 3 starts even though thread 2 has already finished
   the dep graph of mutex creates a circular dep.

   ```
   t1: m1 -> m2
   t2: m2 -> m1
   m1 -> m2
      <-
   ```

   Helgrind doesn't actually print any information about thread 2s creation,
   it just notes that an established order of locking was created and
   thread 3 is violating it.

5. So `main-deadlock-global.c` doesn't actually deadlock (which yes changing the name
   would give this away but I don't like that it's called that).
   Helgrind still reports issues, why? Because it is tracking the order of lock/unlock
   of different mutex address spaces. So because thread 1s order is different than
   thread 2, it still flags a possible issue.

6. Well, there are lots of issues here:

- The `done` variable has no lock so it isn't thread safe for multiple threads. Two threads
  could (but are not here) try to write at the same time, two threads are trying to read/write
  at the same time though.
- The main thread wastes cpu cycles spinning, could've just used join to wait until the worker
  is finished

7. Helgrind races the issue of a data race complaining about no locks held and one thread
   trying to write to `0x4004034` while thread 1 is trying to read saying the read may not be
   accurate. Helgrind does a good job of reporting the lack of a lock.

8. `main-signal-cv.c` does the following:

**How it works:**

- Initializes a struct that has a shared mutex lock and `pthread_cond_t` as well
  as a done flag.
- Creates a thread with a worker, main thread then uses the `signal_wait` function
  to wait for the other thread by locking the mutex lock (if it's already locked it will
  block there) and then if it is not locked it will begin its conditional wait.
- The worker thread does its print and then calls signal done which updates the `done`
  flag and broadcasts the signal to **only** one thread. This is an important distinction
  `pthread_cond_signal` only signals to one thread, there is only one other thread though
  so this works.
- If the main thread had been waiting it would be signled to resume, it resumes control of
  the mutex lock rechecks its while condition of `s->done == 0` which would evaluate to false,
  and then calls the mutex unlock returning to finally print "this should print last\n".

**Improvements**

- Great improvements over the previous code are many
- No pointless CPU cycles with `while`, when `pthread_cond_wait` is called it effectively puts that
  thread to sleep and doesn't wake it until another thread signals that it should re evaluate its
  condition variable.
- No race conditions: since mutex and signals are used appropriately, there are not multiple threads
  trying to read/write variables at once

9. Helgrind correctly does not report any errors.
