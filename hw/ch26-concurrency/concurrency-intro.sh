#!/usr/bin/env bash

configs=(
    #  loop.s:
    # .main
    # .top
    # sub  $1,%dx
    # test $0,%dx
    # jgte .top
    # halt
    # 1) single thread with interrupt every 100 secs
    # interrupt doesn't really do much, this subtracts by 1 and skips the jump
    "Q1|Question 1|-t 1 -p loop.s -i 100 -R dx -c"
    # 2) Same as above but introduces another thread and starts with reg %dx at 3.
    #  Nothing interesting, interrupt is still really long so it never disturbs a threads work
    "Q2|Question 2|-t 2 -p loop.s -i 100 -R dx -a dx=3 -c"
    # 3) Interrupts at least every 3 seconds, assuming an instruction takes a second to execute
    # eventually the value in the %dx reg gets to 0 and termintaes the loop but takes a while
    # # since additonal threads will reset its value to 3 and that register is shared among other
    # threads
    "Q3|Question 3|-t 3 -p loop.s -i 3 -r -R dx -a dx=3 -c"

    #  looping-race-nolock code
    # critical section
    # mov 2000, %ax  # get 'value' at address 2000
    # add $1, %ax    # increment it
    # mov %ax, 2000  # store it back
    #
    # # see if we're still looping
    # sub  $1, %bx
    # test $0, %bx
    # jgt .top
    #
    # With one thread, addr 2000s value starts with 0, is moved into %ax, incremented and moved back to
    # addr 2000, tests the value in %bx and stops the loop since bx is then less than $0. What you would expect
    "Q4|Question 4|-t 1 -p looping-race-nolock.s -M 2000 -c -R bx,ax"
    # multiple threads
    # with no interrupts (default is 50s), the final value is 6. This is because
    # each thread iterates three times since bx is set to three during each threads start
    "Q5|Question 5|-t 2 -p looping-race-nolock.s -M 2000 -c -R bx,ax -a bx=3"
    # somewhat different not really. I don't like the seed here it's hard to follow what it's doing
    "Q6|Question 6|-t 2 -p looping-race-nolock.s -i 4 -s 2  -M 2000 -c -R bx,ax"
    # interrupts happening every two times cause the instructions to almost repeat themselves and extend
    # the loop much longer.
    # not sure which one to get the right output
    "Q7|Question 7|-t 2 -p looping-race-nolock.s -i 8 -M 2000 -c -R bx,ax"
    "Q8|Question 8|-t 2 -p looping-race-nolock.s -i 8 -M 2000 -c -R bx,ax"
    # run program with two threads, one equal to 1 and another to 0 for %ax
    # the waiter (the one with 0) moves the value in addr 2000 to %cx and if it is not
    # equal to 1 jumps back to start of .watier
    # if %ax holds 1 then moves to signlar which moves 1 to address 2000, this then sets the
    # value for waiters next loop to move into %cx
    "Q9|Question 9|-t 2 -p wait-for-me.s -a ax=1,ax=0 -M 2000 -c -R bx,ax"
    # with the first thread set to be the waiter the program can't move forward until
    # the signal is set by the second thread so an interrupt must occur at some point.
    # otherwise the program will loop forever
    "Q10|Question 10|-t 2 -p wait-for-me.s -a ax=0,ax=1 -M 2000 -c -R bx,ax"
)

shopt -s nullglob
assembly_files=(*.s)

if ((${#assembly_files[@]} == 0)) || [[ ! -f "x86.py" ]]; then
    echo "must have assembly files and x86.py for script to execute in cwd"
    echo "copy from https://github.com/remzi-arpacidusseau/ostep-homework/threads-intro"
    exit 1
fi

names=()
for entry in "${configs[@]}"; do
    IFS='|' read -r name desc args <<<"$entry"
    names+=("$name — $desc")
done

select choice in "${names[@]}"; do
    idx=$((REPLY - 1))
    IFS='|' read -r name desc args <<<"${configs[$idx]}"
    echo "Running $name: $desc"
    eval "./x86.py $args"
    break
done
