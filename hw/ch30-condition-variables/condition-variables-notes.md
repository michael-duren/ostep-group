# Homework (Code)
This homework lets you explore some real code that uses locks and
condition variables to implement various forms of the producer/consumer
queue discussed in the chapter. You’ll look at the real code, run it in
various configurations, and use it to learn about what works and what
doesn’t, as well as other intricacies. Read the README for details.

# Questions
1. Our first question focuses on main-two-cvs-while.c (the working solution). First, study the code. Do you think you have an understanding of what should happen when you run the program?
    Essentially the working solution from the chapter (two CVs + while
    loops), same logic as Figure 30.14, just with real variable names
    and homework plumbing (checkpoint labels, end-of-stream marker) added.

    do_fill(value): puts an item into the shared basket at the next
    open slot (fill_ptr), then advances fill_ptr and bumps num_full.
    Has a sanity check (ensure) that the slot it's about to use is
    actually empty first, if not, the locking logic upstream failed.

    do_get(): takes the next item out of the basket (use_ptr), clears
    that slot, advances use_ptr, and decrements num_full. Sanity check
    here confirms the slot wasn't already empty before taking from it.

    producer(): loops `loops` times. Each iteration: lock the basket,
    while it's completely full (num_full == max) go to sleep on the
    `empty` condition variable, once there's room, do_fill an item,
    signal on `fill` (wake a sleeping consumer), unlock. The p0-p6
    labels mark checkpoints where the homework's sleep-string flags can
    force a pause, used later to stage specific interleavings.

    consumer(): loops until it pulls the END_OF_STREAM marker. Each
    iteration: lock the basket, while it's completely empty (num_full
    == 0) go to sleep on `fill`, once there's something there, do_get
    an item, signal on `empty` (wake a sleeping producer), unlock.
    Returns consumed_count-1 (real items only, excluding the sentinel)
    when it finishes. Same c0-c6 checkpoint labels for forced pauses.
    
2. Run with one producer and one consumer, and have the producer
produce a few values. Start with a buffer (size 1), and then increase
it. How does the behavior of the code change with larger buffers?
(or does it?) What would you predict num_full to be with different buffer sizes (e.g., -m 10) and different numbers of produced items (e.g., -l 100), when you change the consumer sleep string
from default (no sleep) to -C 0,0,0,0,0,0,1?
    `./main-two-cvs-while -p 1 -c 1 -m 1 -l 5 -v`
    `./main-two-cvs-while -p 1 -c 1 -m 10 -l 5 -v`
    NF never exceeds 1 in either case. Without an
    artificial delay, both threads run at full speed and the scheduler
    hands control back and forth in lockstep, so extra buffer slots go
    unused. Buffer size alone doesn't change behavior until something
    actually slows one side down.
    `./main-two-cvs-while -p 1 -c 1 -m 1 -l 5 -v -C 0,0,0,0,0,0,1`
    `./main-two-cvs-while -p 1 -c 1 -m 10 -l 5 -v -C 0,0,0,0,0,0,1`
    Adding `-C 0,0,0,0,0,0,1` (1-second sleep at c6, after the consumer
    unlocks, simulating it "processing" the item) changes this:
    - buffer=1: NF still capped at 1, no room to build backlog
      regardless of how slow the consumer is, physically nowhere to
      put a second item.
    - buffer=10, only 5 items: NF climbs to 5 (producer races ahead
      and produces everything while consumer is stuck on its first
      sleep), capped by ITEM COUNT, not buffer capacity, since there
      wasn't enough work to fill all 10 slots.
    - Predicted for -m 10 -l 100: NF would climb to and stay pinned
      near 10 (the actual buffer max), since 100 items is far more
      than the slow consumer can drain, so this time the ceiling is
      buffer CAPACITY, not item count. Producer spends most of its
      time blocked waiting for a freed slot.
    Bigger buffer only helps concurrency when the thing
    causing delay happens outside the lock (like this c6 sleep). A
    sleep that happens WHILE holding the lock serializes everyone regardless of buffer size.

3. If possible, run the code on different systems (e.g., a Mac and Linux).
Do you see different behavior across these systems?
    skip

4. Let’s look at some timings. How long do you think the following execution, with one producer, three consumers, a single-entry shared buffer, and each consumer pausing at point c3 for a second, will take?
    `./main-two-cvs-while -p 1 -c 3 -m 1 -C 0,0,0,1,0,0,0:0,0,0,1,0,0,0:0,0,0,1,0,0,0 -l 10 -v -t`
    c3 sits right after Cond_wait returns, meaning the sleep happens
    while the consumer is holding the mutex (Cond_wait reacquires the
    lock before returning). So this 1-second pause blocks everyone
    else from touching the lock during that second.

    Predicted ~13 seconds (10 items + 1 sec pause per consumer).
    Buffer size 1 (only one item can ever be in flight at a time), more consumers do not provide more speed here.

    Result: 11-12 seconds
    13 seconds is the ceiling, 10 items plus 3 end-of-stream markers, each a separate wakeup that costs a second if a consumer actually has to sleep for it. In practice you land a bit under that (I saw 11 and 12 across two runs) because whichever handoffs get lucky and don't need to sleep save a second each. 

5. Now change the size of the shared buffer to 3 (-m 3). Will this make
any difference in the total time?
    `./main-two-cvs-while -p 1 -c 3 -m 3 -C 0,0,0,1,0,0,0:0,0,0,1,0,0,0:0,0,0,1,0,0,0 -l 10 -v -t`
    Predicted no real difference vs buffer=1, since the bottleneck
    isn't buffer capacity, it's the lock being held during the c3
    sleep. More storage space doesn't help if nothing can add/remove
    items while being locked down for a full second.

    Result: 11-12 seconds. Confirms buffer size is irrelevant here.

6. Now change the location of the sleep to c6 (this models a consumer taking something off the queue and then doing something with it), again using a single-entry buffer. What time do you predict in this case? 
    if all 3 consumers working at once for 10 items = 3.3 seconds?
    `./main-two-cvs-while -p 1 -c 3 -m 1 -C 0,0,0,0,0,0,1:0,0,0,0,0,0,1:0,0,0,0,0,0,1 -l 10 -v -t`
    sleep at c6 = after Mutex_unlock, so it's lock-free ("processing" happens outside the lock)
    with 3 consumers able to work in parallel, total time is no longer ~1sec x items
    instead it's bounded by whichever consumer ends up doing the most work
    Results: C0 -> 3, C1 -> 3, C2 -> 4, Total time: 4 - 5 seconds
    (not ~10s like Q4/Q5, and not evenly 10/3≈3.3s either -  imbalanced load on C2 sets the pace)

7. Finally, change the buffer size to 3 again (-m 3). What time do you
predict now?
    prediction: no meaningful change, since buffer size wasn't the bottleneck in Q6
    `./main-two-cvs-while -p 1 -c 3 -m 3 -C 0,0,0,0,0,0,1:0,0,0,0,0,0,1:0,0,0,0,0,0,1 -l 10 -v -t`
    Results: C0 -> 4, C1 -> 3, C2 -> 3, Total time: 4 - 5 seconds
    confirms: once sleep moved outside the lock, buffer size became irrelevant
    the real constraint is how evenly work distributes across consumer threads

8. Now let’s look at main-one-cv-while.c. Can you configure
a sleep string, assuming a single producer, one consumer, and a
buffer of size 1, to cause a problem with this code?
    main-one-cv-while.c uses ONE shared cv for both producer-wait and consumer-wait
    with -p 1 -c 1, only one thread of each type exists
    "signal wakes wrong type" can't cause a real problem here, since there's no ambiguity in who gets woken
    conclusion: this bug needs multiple threads of at least one type to actually manifest

9. Now change the number of consumers to two. Can you construct
sleep strings for the producer and the consumers so as to cause a
problem in the code?
    `timeout 10 ./main-one-cv-while -p 1 -c 3 -m 1 -l 6 -v -t`
    main-one-cv-while.c with -p 1 -c 3 -m 1 -l 6 (no sleep strings, just raw scheduling)
    with only 2 threads (1 producer, 1 consumer) the bug can't manifest (Q8)
    with 3 consumers competing for 1 shared cv, it hung on the very first try
    Results: trace stops after only 2 of 3 expected end-of-stream markers delivered
    no "Consumer consumption" or "Total time" ever printed, timeout had to kill it
    cause: a signal meant to wake the last waiting consumer woke a different,
    already-satisfied one instead (a consumer, not the intended target) — that
    thread found nothing to do and went back to sleep, and since production
    was already finished, no further signal ever came to rescue the stuck one
    proves: sharing one cv between two logically different wait conditions
    (space-available vs data-available) is unsafe

10. Now examine main-two-cvs-if.c. Can you cause a problem to
happen in this code? Again consider the case where there is only
one consumer, and then the case where there is more than one.
    `./main-two-cvs-if -p 1 -c 4 -m 1 -l 10 -v -t`
    main-two-cvs-if.c, separate empty/fill cvs (Q8/9 bug fixed) but if instead of while
    -p 1 -c 4 -m 1 -l 10, no sleep strings needed
    if checks the condition once and never rechecks after waking up
    a woken consumer can lose a race for the item to a different consumer
    that got the lock first, then proceeds anyway since if doesn't recheck
    Results: crashed on the first run
    "error: tried to get an empty buffer"
    proves: while isn't just defensive style, it's required correctness
    
11. Finally, examine main-two-cvs-while-extra-unlock.c. What
problem arises when you release the lock before doing a put or a
get? Can you reliably cause such a problem to happen, given the
sleep strings? What bad thing can happen?
    `./main-two-cvs-while-extra-unlock -p 3 -c 3 -m 3 -l 20 -v -t`
    `./main-two-cvs-while-extra-unlock -p 4 -c 4 -m 2 -l 200 -t`
    main-two-cvs-while-extra-unlock.c releases the mutex before do_fill/do_get,
    then reacquires it just to signal — the actual buffer mutation happens unprotected
    tried -p 3/4 -c 3/4, -m 1-3, -l 20-500, multiple runs
    Results: no crash or totals mismatch observed empirically on this machine
    doesn't mean the code is correct — do_fill/do_get are only a few fast
    instructions, so the unprotected window is narrow, and modern CPUs/schedulers
    don't always hit it even under real contention
    the bug is proven by code inspection (shared state mutated with no lock held),
    not by reproducing a crash — this is the same lesson as ch 26's race conditions:
    "it worked when I ran it" was never a valid correctness argument






Q4-7 are one experiment: same 1-second sleep, same buffer change (1 → 3),
sleep relocated from before the lock releases (c3) to after (c6).
Buffer size made zero measurable difference in either case
(Q4 11.00s vs Q5 11.13s; Q6 4.53s vs Q7 4.68s) — the entire ~10s → ~4.5s
change came from moving the sleep, not from the buffer. 

Lesson: don't assume a bigger buffer buys concurrency just because you moved slow work outside
the lock
bigger buffer helps when production can outpace how fast slots get freed, and the buffer is the thing actually filling up and blocking the producer. It's a shock absorber for bursts of high activity, it lets the producer keep working ahead of schedule instead of stalling every time, up to whatever capacity you gave it.