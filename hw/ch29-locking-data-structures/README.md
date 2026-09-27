# Chapter 29: Lock-based Concurrent Data Structures

How do we add locks to specific data structures that we define, such that they are usable across threads safely, or **thread-safe**?

## Counters

Basic counter without locks:
```pseudo
class counter
  int value

  init() 
    this.value = 0

  inc()
    this.value++
  get()
    return this.value
```

Counter with locks:
```pseudo
class counter
  int value
  mutex lock

  init() 
    this.value = 0
    lock.init()

  inc()
    lock.lock()
    this.value++
    lock.unlock()

  get()
    lock.lock()
    int rc = this.value
    lock.unlock()
    return rc
```

^^--> EZPZ <--^^

Now your threads don't have to care if they are operating async or not, just call the method and the module handles the mutex for you. 
> As a C# dev I am thinking of built-in thread-safe structures like ConcurrentBag which simply handle these things for you. I now understand the point of this

> Also omg I cannot with this guy constantly referencing images that are displayed on different pages

This simplistic approach can have performance problems. This specifically does not scale well with a workload across multiple threads/processors.

If you can get threads across multiple processors just as quickly as a single thread does on one, you have achieved __perfect scaling__.

**The reasons this is slow**
1. the additional incurred cost of threads coordinating. A single thread never really reaches the "spin" or "sleep" areas of mutex logic, despite still working with the mutex, it is simply less expensive when there is no concurrency.
2. cache coherence between threads -- since the same physical memory value is being updated across multiple cores, the CPU is constantly busting cache. Caching isn't actually faster if you bust it more often than you don't!

### Scalable Counting

a very cool alternative approach is using something called an __approximate counter__
- represent a single logical counter via `n` physical counters (`n` == # of cpu cores), `n+1` memory locations including global counter for reads

Common approach would be to pick a threshold for each local counter to flush and increment the global counter. ex. local1 hits 5, resets to 5 and adds 5 to global.

Obviously this means the global counter will lag behind the true logical sum of all the counters, so you now have a new problem to solve, which you can tune by adjusting the threshold
- High threshold == moar faster
- Low threshold == moar accurate

---

## Concurrent Linked Lists

Basic implementation. Notice the exception path on alloc needs to unlock the mutex. 
Studies show that bugs exist so this is bad I guess

```pseudo
struct node
  int key
  node next

class list 
  node head
  mutex lock

  init()
    this.head = null
    lock.init()
  
  ins(int key)
    lock.lock()
    node new = alloc(node)
    if new == null
      lock.unlock()
      return err("alloc")
    new.key = key
    new.next = this.head
    this.head = new
    lock.unlock()

  lookup(int key)
    lock.lock()
    node curr = this.head
    while curr 
      if curr.key == key
        lock.unlock()
        return curr
      curr = curr.next

    lock.unlock()
    return err("failed")
```

Rewritten for modern audiences:

```pseudo
struct node
  int key
  node next

class list 
  node head
  mutex lock

  init()
    this.head = null
    lock.init()
  
  ins(int key)
    node new = alloc(node)
    if !new 
      return err("alloc")
    new.key = key

    // scope lock down to critical section only
    lock.lock()
    new.next = this.head
    this.head = new
    lock.unlock()

  lookup(int key)
    node rv = null // its my language structs can be null here it is godless
    lock.lock()
    node curr = this.head
    while curr 
      if curr.key == key
        rv = curr
      curr = curr.next

    lock.unlock()
    return rv ?? err("failed")
```

1 unlock per function, awesome, no branching path unlocks to worry about

### Scaling Linked Lists

Similar scaling issues across CPUs to the counter.

**Lock Coupling** is the practice of storing one lock per list node.
- Additional mechanism required on top of this to smartly traverse the locks 
- basically just hold one lock, grab the next lock, release the prev lock.

However, whether this speeds up the list operations is relatively inconclusive, if not simply false. 
While it saves you from locking the entire list at once, enabling other threads to access other parts of the list, it incurs additional overhead due to all the additional lock and release operations.

> So I'm not sure why the book even brought it up. Turns out you cannot scale a linked list for concurrency ok moving on

### Concurrent Queues

The same general single-lock pattern could be applied here. Skipping

**The Michael and Scott Concurrent Queue** is a 2-lock pattern that separately locks the enqueue and dequeue operations (ie. the start and end of the queue)
- uses dummy node that is always in queue to ensure there is always a piece of data being pointed to by head/tail
- the dummy node avoids the race condition of the head and tail locks guarding the same piece of data.
- when queue empty, they do both point at the same dummy node, but it is not protected data and is only ever modified by setting its .next to a real queue item


### Concurrent Hash Table

A good design here uses a lock per **hash bucket** (colocated grouping of hash entries, _usually_ just one item)
One doesn't really "traverse" a hashtable and it contains a bunch of dispersed data, it just lends itself more easily to a practical concurrency pattern than the other structures.

