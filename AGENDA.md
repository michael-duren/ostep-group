# OSTEP Book Club — 13-Week Schedule

**Book:** Operating Systems: Three Easy Pieces (free at <https://pages.cs.wisc.edu/~remzi/OSTEP/>)

**Time budget:** ~4 hrs/week → 2.5–3 hrs reading + homework on your own, ~1 hr meeting

**Meeting format:** ~30 min presenter walkthrough of the chapters, ~30 min homework discussion

**Presenters:** rotate weekly — presenter summarizes key ideas, demos their homework solutions, and brings 2–3 discussion questions. Everyone else reads and attempts the homework before meeting.

> Homework simulators and code are at <https://github.com/remzi-arpacidusseau/ostep-homework>

---

## Part 1: Virtualization — CPU

### Week 1 — Week of August 3: Processes & the OS

- Ch 1–2: Dialogue + Introduction to Operating Systems
- Ch 4: The Abstraction: The Process
- Ch 5: Process API (fork, exec, wait)
- Ch 6: Limited Direct Execution

**HW:** `process-run.py` simulator; write C programs using fork/exec/wait (Ch 5 coding questions)

### Week 2 — Week of August 10: CPU Scheduling

- Ch 7: Scheduling: Introduction (FIFO, SJF, STCF, RR)
- Ch 8: Multi-Level Feedback Queue
- Ch 9: Lottery & Proportional-Share Scheduling
- Ch 10: Multiprocessor Scheduling

**HW:** `scheduler.py`, `mlfq.py`, `lottery.py`

---

## Part 1: Virtualization — Memory

### Week 3 — Week of August 17: Address Spaces & Translation

- Ch 13: The Abstraction: Address Spaces
- Ch 14: Memory API (malloc/free)
- Ch 15: Address Translation (base & bounds)
- Ch 16: Segmentation

**HW:** Ch 14 coding questions (use valgrind/gdb), `relocation.py`, `segmentation.py`

### Week 4 — Week of August 24: Free Space & Paging

- Ch 17: Free-Space Management
- Ch 18: Paging: Introduction
- Ch 19: Translation Lookaside Buffers

**HW:** `malloc.py`, `paging-linear-translate.py`, TLB measurement program (Ch 19)

### Week 5 — Week of August 31: Advanced Paging & Swapping

- Ch 20: Paging: Smaller Tables (multi-level page tables)
- Ch 21: Swapping: Mechanisms
- Ch 22: Swapping: Policies (FIFO, LRU, clock)

**HW:** `paging-multilevel-translate.py`, `paging-policy.py`

### Week 6 — Week of September 7: Real VM Systems + Concurrency Kickoff

- Ch 23: Complete Virtual Memory Systems (VAX & Linux)
- Ch 25–26: Dialogue + Concurrency: An Introduction
- Ch 27: Thread API

**HW:** Ch 26 `x86.py` race-condition exercises; write first pthread programs

---

## Part 2: Concurrency

### Week 7 — Week of September 14: Locks

- Ch 28: Locks
- Ch 29: Lock-based Concurrent Data Structures

**HW:** `x86.py` lock exercises; build & benchmark a concurrent counter and list

### Week 8 — Week of September 21: Condition Variables & Semaphores

- Ch 30: Condition Variables
- Ch 31: Semaphores

**HW:** producer/consumer, reader-writer locks, dining philosophers (Ch 31 coding)

### Week 9 — Week of September 28: Bugs & Event-Based Concurrency

- Ch 32: Common Concurrency Problems (atomicity/ordering bugs, deadlock)
- Ch 33: Event-Based Concurrency

**HW:** deadlock exercises (vector-deadlock programs); simple select/epoll server

---

## Part 3: Persistence

### Week 10 — Week of October 5: Devices, Disks & Files

- Ch 36: I/O Devices
- Ch 37: Hard Disk Drives
- Ch 38: RAID
- Ch 39: Interlude: Files and Directories

**HW:** `disk.py`, `raid.py`; Ch 39 file/directory API coding (stat, ls, recursive search)

### Week 11 — Week of October 12: File System Implementation

- Ch 40: File System Implementation (vsfs)
- Ch 41: Locality and the Fast File System
- Ch 42: Crash Consistency: FSCK and Journaling

**HW:** `vsfs.py`, `ffs.py`, `fsck.py`

### Week 12 — Week of October 19: Modern Storage

- Ch 43: Log-Structured File Systems
- Ch 44: Flash-Based SSDs
- Ch 45: Data Integrity and Protection

**HW:** `lfs.py`, `ssd.py`, `checksum.py`

---

## Part 4: Distributed Systems + Wrap-Up

### Week 13 — Week of October 26: Distribution & Retrospective

- Ch 48: Distributed Systems (RPC)
- Ch 49: Sun's Network File System (NFS)
- Ch 50: The Andrew File System (AFS)

**Final discussion:** biggest takeaways, what to build/read next (e.g., xv6, the OSTEP projects, or MIT 6.824 for more distributed systems)

---

## Presenter Rotation

Fill in names once your roster is set. With N members, each person presents every N weeks.

| Week | Date   | Topic                                 | Presenter           |
| ---- | ------ | ------------------------------------- | ------------------- |
| 1    | Aug 9  | Processes & the OS                    | Michael             |
| 2    | Aug 16 | CPU Scheduling                        | Kaleb               |
| 3    | Aug 23 | Address Spaces & Translation          | Charlie             |
| 4    | Aug 30 | Free Space & Paging                   | Christine           |
| 5    | Sep 6  | Advanced Paging & Swapping            | NA                  |
| 6    | Sep 13 | Real VM Systems + Concurrency Kickoff | Remi/Michael        |
| 7    | Sep 20 | Locks                                 | Kaleb               |
| 8    | Sep 27 | CVs & Semaphores                      | Charlie             |
| 9    | Oct 4  | Bugs & Events                         | Christine           |
| 10   | Oct 11 | Devices, Disks & Files                | Remi                |
| 11   | Oct 18 | File System Implementation            | Michael             |
| 12   | Oct 25 | Modern Storage                        | Plan group project  |
| 13   | Nov 1  | Distributed Systems & Wrap-Up         | Standup for project |

---

## Tips

- The short "dialogue" chapters (3, 11, 12, 24, 25, 34, 35, 46, 47, 51) take 2 minutes each — read them for fun, don't schedule around them.
- Weeks 7–9 (concurrency) benefit most from live coding during meetings — races and deadlocks are best understood by watching them happen.
- If a week runs long, Ch 23, 33, and 45 are the safest to skim or cut.
- Consider a shared repo where everyone commits homework solutions before each meeting.
