# Chapter 28: Locks

## Basic Lock Structure

We use locks (called 'mutex' in POSIX) to prevent concurrently running threads from touching the same pieces of shared state at the same time. This prevents a class of bug called a race condition.

A 'lock' (or 'mutex' in POSIX) is a struct with two primary related functions (in other languages these would be methods):
- `lock()` attempts to acquire the lock, otherwise waits until lock is aquirable
- `unlock()` releases an acquired lock

This provides some minimal control over the scheduler. 
    - Ideally, threads literally sleep while waiting for locks, no work is done until thread wakes. More on this later.

Pthread locks (an implementation of mutex) pass around specific locks for fine-grained control over locking. If they didn't, you'd only have one lock available per process.
```c
// Before splitting to separate threads:
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int my_special_var;

// Inside individual threads
Pthread_mutex_lock(&lock);
my_special_var += 1;
Pthread_mutex_unlock(&lock);
```

## Lock Evaluation Criteria

1. Correctness -- Does it provide mutual exclusion? 
2. Fairness -- does the lock have the potential to starve threads under certain circumstances?
3. Performance -- what does the time overhead look like?
    - Overhead can be measured in three primary contexts:
        - No contention -- single thread runs and releases the lock
        - Multiple thread contention on single CPU
        - Multiple thread contention across multiple CPU



## Lock implementation approaches 

### Disabling interrupts
Bad, limited, doesn't work on multiple cpus, prone to DOS attack

### Spin-Waiting
Purely use userspace-available code to invent a locking mechanism.

**Bad**, cannot atomically guarantee mutual exclusion.

### Spin-lock
This one actually works, but requires dedicated atomic assembly instructions which execute multiple actions at once.

#### test-and-set
Hardware supports atomic `test-and-set` instruction
- Accepts a memory location and an immediate value
- Sets the immediate value to the memory location
- returns(?) the previous value at memory location


```c
int TestAndSet(int *old_ptr, int new) {
    int old = *old_ptr;
    *old_ptr = new;
    return old;
}
```

```c
void lock(lock_t *lock) {
    while (TestAndSet(&lock->flag, 1) == 1) {} // spin-wait
}
    
void unlock(lock_t *lock) {
    lock->flag = 0;
}
```

Unfortunately spin locks do not guarantee **fairness** unless the hardware has a "preemptive" (timer-based) scheduler 

They also do not **perform** well in single-cpu systems


#### compare-and-swap

Unlike `test-and-set`, `compare-and-swap` accepts an expected value and compares before assigning the new value. 
Otherwise these two are used in a similar way. For spin locks the implementation is identical, but they have more powerful applications in other approaches.


```c
int CompareAndSwap(int *ptr, int expected, int new) {
    int original = *ptr;
    if (original == expected) {
        *ptr = new;
    }
    return original;
}
```

#### load-linked && store-conditional

A pair of instructions that act like a split of the `compare-and-swap` code. Pretty similar implementation but requires more logic on the lock implementation to stick them together.
```c
int LoadLinked(int *ptr) {
    return *ptr;
}

int StoreConditional(int *ptr, int value) {
    if (//no update to *ptr since LL to this addr) {
        *ptr = value;
        return 1;
    }
    else {
        return 0;
    }
}

```

#### fetch-and-add 

Automatically increment a value, return previous value. You can imagine the code for this I am sure.

This can be used to build a different type of lock entirely using a "ticket and turn" pattern:
- thread wishes to acquire lock
- -> calls fetch-and-add
    - internal counter is the "turn" counter, return value is the "ticket" for that request
- -> thread gets the lock when it is his turn 

## More Performant Locking

I mentioned spin locks have performance issues earlier, this is a primary reason they are not used often.

The reason for this is that the threads waiting for a lock do not truly sleep -- they _spin_, wasting a full timeslice each time they are scheduled.

One approach to fix this is to include an OS-level primitive called `yield`, which allows a process to deschedule itself when it knows it is waiting.
- This _improves_ performance but doesn't fully address the issue, as the scheduler is still wasting time scheduling the thread.
- This matters more when there are a large number of threads "yielding", as each thread still needs to be scheduled and continue execution at least momentarily.

A better approach is a true `sleep`
- Just let the OS scheduler know more about the concurrent processes and locks, and make scheduling decisions accordingly. 

### Solaris Park and Unpark

Example implementation of sleep by Solaris
process 1: 
```c
queue_add(my_queue, gettid()); // here is my PID please add me to the queue
setpark(); // omg I'm gonna park nobody touch the guard til I do
guard = 0; // other processes can have the lock now
park(); // yield and stop scheduling me` 
```
process 2: 
```c
unpark(next_queue_item(my_queue)); // OK I'm finished, start scheduling PID 1 again`
```

### Linux futex

`futex_wait(address, expected)` puts calling thread to sleep if address == expected, else proceeds
`futex_wake(address)` releases the next thread on the queue

This, and other approaches mentioned, use what is called a **Two-Phase** approach to locking.
- Phase 1: Can I have the lock right now? (if so, skips phase 2)
- Phase 2: goes to sleep waiting for lock
