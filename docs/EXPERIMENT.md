# Existing experiment: reproducibility guide

## Workflow

This cleanup preserves the previously implemented protocol, not a rewritten implementation. On an authorized Ajorpaz lab CPU pair, run `bash tools/run_calibrated.sh ATTACKER_CPU VICTIM_CPU results/NEW_RUN_NAME` with two permitted **distinct** logical CPUs. Use a new directory to prevent overwriting evidence. Scripts require Linux x86-64, C compiler, shell tools, Python 3, NumPy and Matplotlib for plots.

The synthetic, student-controlled victim is launched via fork and shares a physical cache line in anonymous shared memory. The timed probe is **CLFLUSH plus fencing**, not Flush+Reload; timestamp units are TSC ticks. Attacker/victim process placement and setup information are recorded.

The protocol discards 1,000 warmups, calibrates with 10,000 balanced samples per bit class, searches threshold and direction on calibration **only**, freezes the chosen rule, then collects ten fresh balanced 10,000-trial runs per condition. The conditions are: `signal` (victim access to target on bit 1), `unrelated_line` (access a different line/page, but cache set not verified), and `no_victim` (no victim handshake). Each trial resets the target. The independent `ff_bench` diagnostic threshold must not replace the victim-matched frozen threshold. The condition order is fixed and may drift.

## File dictionary (each `results/<host>/`)

| File | Meaning |
|---|---|
| `calibration.csv`, `threshold.txt` | Unbinned calibration measurements; frozen cutoff and polarity |
| `trials.csv`, `summary.csv` | Unbinned labeled held-out trials; ten-run confusion counts and rates per condition |
| `report.md`, `resources.csv` | Accuracy and variability, FPR/FNR, timing and throughput, setup cost/RSS |
| `figures/` | Previously generated timing-distribution, error, accuracy and memory plots |
| `metadata.txt`, `placement.txt`, `topology.txt`, `lscpu.txt`, other TXT | Recorded machine configuration, stdout/errors and build environment |
| `source.c`, `ff_common.h`, `ff_experiment`, `sha256.txt` | Immutable source/binary snapshots and checksums |
| `verification.txt` | Previous verification record; rerun `python3 tools/verify_evidence.py results/<host>` to check |

Throughput measures collection batches, including reset/handshake but not CSV writing. End-to-end time, empirical information estimates and true leakage capacity are different concepts. Memory RSS is per process and shared pages are counted twice if summing. Per-trial probe p50/p95 uses TSC ticks; mean trial microseconds is amortized batch duration. Keep uncertainties and negative findings.
