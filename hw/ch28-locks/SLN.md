# Homework Problems

I am only presenting a couple of these for brevity.

### 1. Examine `flag.s`. This code "implements" locking with a single memory flag. Can you understand the assembly?

Answer: Yes I can understand it because you literally wrote it in comments in plain english

This is a simple naiive approach -- it loops from the `mov flag, %ax` line to the `jne .acquire` line until the number at address `flag` is 0.

Biggest flaw based on takeaways from this chapter is the fact that loading the flag to a register and reading the flag are two separate operations.
It lacks __atomicity__ and cannot guarantee that the flag was not acquired (due to hardware interrupt + a sister thread being scheduled) between those two operations.
Essentially the same exact race condition you are trying to prevent in `# critical section` is possible in the `.acquire` section.


```asm
.var flag
.var count

.main
.top

.acquire
mov  flag, %ax      # get flag
test $0, %ax        # if we get 0 back: lock is free!
jne  .acquire       # if not, try again
mov  $1, flag       # store 1 into flag

# critical section
mov  count, %ax     # get the value at the address
add  $1, %ax        # increment it
mov  %ax, count     # store it back

# release lock
mov  $0, flag       # clear the flag now

# see if we're still looping
sub  $1, %bx
test $0, %bx
jgt .top	

halt
```

### 2. When you run with the defaults, does `flag.s` work? Use the -M and -R flags to trace variables and registers (and turn on -c to see their values). Can you predict what value will end up in `flag`?

Answer: 
Yes flag.s works with defaults, as the defaults have a long interrupt allowing for no race condition in the first place; it never locks.
The value that ends up in `flag` is simply 0 after execution, as the immediate value $0 being directly copied into it is the last operation on the `flag` variable in the program.

`❯ ./x86.py -p flag.s -M 2000 -R ax,bx -c`

```
 2000      ax    bx          Thread 0                Thread 1

    0       0     0
    0       0     0   1000 mov  flag, %ax
    0       0     0   1001 test $0, %ax
    0       0     0   1002 jne  .acquire
    0       0     0   1003 mov  $1, flag
    0       0     0   1004 mov  count, %ax
    0       1     0   1005 add  $1, %ax
    0       1     0   1006 mov  %ax, count
    0       1     0   1007 mov  $0, flag
    0       1    -1   1008 sub  $1, %bx
    0       1    -1   1009 test $0, %bx
    0       1    -1   1010 jgt .top
    0       1    -1   1011 halt
    0       0     0   ----- Halt;Switch -----  ----- Halt;Switch -----
    0       0     0                            1000 mov  flag, %ax
    0       0     0                            1001 test $0, %ax
    0       0     0                            1002 jne  .acquire
    0       0     0                            1003 mov  $1, flag
    0       1     0                            1004 mov  count, %ax
    0       2     0                            1005 add  $1, %ax
    0       2     0                            1006 mov  %ax, count
    0       2     0                            1007 mov  $0, flag
    0       2    -1                            1008 sub  $1, %bx
    0       2    -1                            1009 test $0, %bx
    0       2    -1                            1010 jgt .top
    0       2    -1                            1011 halt

```
Bonus, we can break the lock using just the right interrupt interval:
`❯ ./x86.py -p flag.s -M 2000 -R ax,bx -c -i 3`

```
 2000      ax    bx          Thread 0                Thread 1

    0       0     0
    0       0     0   1000 mov  flag, %ax
    0       0     0   1001 test $0, %ax
    0       0     0   1002 jne  .acquire
    0       0     0   ------ Interrupt ------  ------ Interrupt ------
    0       0     0                            1000 mov  flag, %ax
    0       0     0                            1001 test $0, %ax
    0       0     0                            1002 jne  .acquire
    0       0     0   ------ Interrupt ------  ------ Interrupt ------
    0       0     0   1003 mov  $1, flag
    0       0     0   1004 mov  count, %ax
    0       1     0   1005 add  $1, %ax
    0       0     0   ------ Interrupt ------  ------ Interrupt ------
    0       0     0                            1003 mov  $1, flag
    0       0     0                            1004 mov  count, %ax
    0       1     0                            1005 add  $1, %ax
    0       1     0   ------ Interrupt ------  ------ Interrupt ------
    0       1     0   1006 mov  %ax, count
    0       1     0   1007 mov  $0, flag
    0       1    -1   1008 sub  $1, %bx
    0       1     0   ------ Interrupt ------  ------ Interrupt ------
    0       1     0                            1006 mov  %ax, count
    0       1     0                            1007 mov  $0, flag
    0       1    -1                            1008 sub  $1, %bx
    0       1    -1   ------ Interrupt ------  ------ Interrupt ------
    0       1    -1   1009 test $0, %bx
    0       1    -1   1010 jgt .top
    0       1    -1   1011 halt
    0       1    -1   ----- Halt;Switch -----  ----- Halt;Switch -----
    0       1    -1   ------ Interrupt ------  ------ Interrupt ------
    0       1    -1                            1009 test $0, %bx
    0       1    -1                            1010 jgt .top
    0       1    -1                            1011 halt
```

Specifically observe the different ending state of `ax`
