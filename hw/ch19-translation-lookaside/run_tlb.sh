#!/bin/bash
#
# run_tlb.sh
#
# Sweeps the number of pages touched by tlb.c from a starting value up to a
# maximum, doubling each iteration (per OSTEP Ch.19 Question 3), and records
# the average per-access cost at each size to a data file for graphing
# (see Question 4).
#
# Usage:
#   ./run_tlb.sh [min_pages] [max_pages] [trials] [repeats] [outfile]
#
#   min_pages  - smallest page count to test          (default: 1)
#   max_pages  - largest page count to test            (default: 4096)
#   trials     - inner repeat count passed to tlb.c    (default: 200000)
#   repeats    - independent re-runs per page count,   (default: 5)
#                the median of which is kept, to filter out noise
#                from scheduling hiccups / interrupts (see "how many
#                trials are needed?" discussion below)
#   outfile    - where to write results                (default: results.dat)
#
# Output format (results.dat): three whitespace-separated columns
#   <num_pages>  <ns_per_access_median>  <all_repeat_values...>
# suitable for feeding directly into gnuplot/ploticus/Python.

set -euo pipefail

MIN_PAGES=${1:-1}
MAX_PAGES=${2:-4096}
TRIALS=${3:-200000}
REPEATS=${4:-5}
OUTFILE=${5:-results.dat}
BINARY=./tlb

if [ ! -x "$BINARY" ]; then
    echo "error: $BINARY not found or not executable (build it first: gcc -O2 -o tlb tlb.c)" >&2
    exit 1
fi

# Returns the median of the numbers passed as arguments.
median() {
    local sorted
    sorted=$(printf '%s\n' "$@" | sort -n)
    local n
    n=$(printf '%s\n' "$@" | wc -l)
    local mid=$(( (n + 1) / 2 ))
    echo "$sorted" | sed -n "${mid}p"
}

echo "# num_pages  ns_per_access_median  [individual repeat values]" > "$OUTFILE"

pages=$MIN_PAGES
while [ "$pages" -le "$MAX_PAGES" ]; do
    values=()
    for ((r = 0; r < REPEATS; r++)); do
        # tlb.c prints: "<pages> <trials> <ns_per_access>" -- take the 3rd field
        val=$("$BINARY" "$pages" "$TRIALS" | awk '{print $3}')
        values+=("$val")
    done

    med=$(median "${values[@]}")
    echo "$pages $med ${values[*]}" >> "$OUTFILE"
    echo "pages=$pages -> median ${med} ns/access (samples: ${values[*]})"

    pages=$(( pages * 2 ))
done

echo "Done. Results written to $OUTFILE"
