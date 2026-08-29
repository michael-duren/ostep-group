# Chapter 19: Translation Lookaside Buffers

Homework for OSTEP Chapter 19 "Paging: Faster Translations (TLBs)".

## Files

- [tlb.c](tlb.c) - sweeps `num_pages` distinct pages, `num_trials` times, and
  prints `<num_pages> <num_trials> <avg_ns_per_access>`. Answers to the
  chapter's discussion questions (clock resolution, warmup, compiler
  optimization, etc.) are documented inline as comments near the relevant
  code.
- [run_tlb.sh](run_tlb.sh) - builds a `results.dat` sweep by doubling the page
  count from a minimum to a maximum, running several repeats per size, and
  keeping the median to filter out scheduling noise.
- [plot_tlb.py](plot_tlb.py) - reads `results.dat` and renders `tlb_graph.png`
  in the style of Figure 19.5 (ns/access vs. number of pages, log2 x-axis).
- `results.dat`, `tlb_graph.png` - sample output from a run on this machine.

## Usage

```sh
gcc -o tlb tlb.c
./tlb <num_pages> <num_trials>          # single measurement

./run_tlb.sh [min_pages] [max_pages] [trials] [repeats] [outfile]
./run_tlb.sh 1 4096 200000 5 results.dat

python3 plot_tlb.py [results.dat] [output.png]
```

`run_tlb.sh` defaults to a 1-4096 page sweep, 200000 trials per measurement,
and 5 repeats (median kept), writing to `results.dat`.

## What to look for

The resulting graph should show a step pattern: per-access time is low and
flat while the working set fits in the TLB, jumps once it exceeds the TLB's
reach (an ITLB/DTLB or L1/L2 TLB size boundary), and jumps again if the
working set later outgrows the cache.
