#!/usr/bin/env python3
"""
plot_tlb.py

Reads the results.dat file produced by run_tlb.sh and produces a graph in
the style of OSTEP Figure 19.5 ("Discovering TLB Sizes and Miss Costs"):
average time per access (ns) on the y-axis, number of pages touched
(log2 scale) on the x-axis.

Usage:
    python3 plot_tlb.py [results.dat] [output.png]
"""

import sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

INFILE = sys.argv[1] if len(sys.argv) > 1 else "results.dat"
OUTFILE = sys.argv[2] if len(sys.argv) > 2 else "tlb_graph.png"

pages, ns = [], []
with open(INFILE) as f:
    for line in f:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        pages.append(int(parts[0]))
        ns.append(float(parts[1]))

fig, ax = plt.subplots(figsize=(7, 4.5), facecolor="#fcfcfb")
ax.set_facecolor("#fcfcfb")

ax.plot(
    pages, ns,
    color="#2a78d6",
    linewidth=2,
    marker="o",
    markersize=6,
    markerfacecolor="#2a78d6",
    markeredgecolor="#fcfcfb",
    markeredgewidth=1,
)

ax.set_xscale("log", base=2)
ax.xaxis.set_major_formatter(ticker.ScalarFormatter())
ax.xaxis.set_major_locator(ticker.LogLocator(base=2, subs=[1.0], numticks=20))

ax.set_xlabel("Number Of Pages", color="#0b0b0b")
ax.set_ylabel("Time Per Access (ns)", color="#0b0b0b")
ax.set_title("TLB Size Measurement", color="#0b0b0b")

ax.grid(True, which="major", color="#e1e0d9", linewidth=0.8)
ax.set_axisbelow(True)

for spine in ["top", "right"]:
    ax.spines[spine].set_visible(False)
for spine in ["left", "bottom"]:
    ax.spines[spine].set_color("#c3c2b7")

ax.tick_params(colors="#898781")

fig.tight_layout()
fig.savefig(OUTFILE, dpi=150)
print(f"Wrote {OUTFILE}")
