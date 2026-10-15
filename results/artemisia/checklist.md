# Artemisia experiment checklist

Collected 2026-10-15 on artemisia.ece.ncsu.edu, using the same source protocol and CPU 0/1 placement as Skylark. CPU 0/core 0 and CPU 1/core 1 share socket 0; inherited affinity permits both.

- [x] Calibration: preserved user benchmark calibration.txt and bench_calibration.csv; independently collected victim-matched calibration.csv (10,000/class), frozen threshold.txt before tests. Rule: ticks <= 248 predicts bit 1.
- [x] Repeated trials: trials.csv and summary.csv contain 10 runs of 10,000 balanced trials for each of three conditions (300,000 total).
- [x] CPU/configuration: hostname.txt, lscpu.txt, topology.txt, allowed_cpus.txt, placement.txt, kernel.txt, compiler.txt, metadata.txt. Inspected disassembly.txt for CLFLUSH/MFENCE and RDTSCP/LFENCE.
- [x] Controls: unrelated_line and no_victim collected. Different-line cache-set independence remains unverified; no-victim omits handshake.
- [x] Additional measurements: report.md and resources.csv record errors, throughput, latency, setup, mapping size, and per-process RSS.
- [x] Reproducibility: source.c, ff_common.h, ff_experiment, sha256.txt, raw CSVs, seed in metadata.txt, exit_status.txt = 0; verification.txt passes.
- [x] Graphs: figures/ contains accuracy_by_condition, errors_and_throughput, timing_distributions, setup_and_memory, each as PNG and PDF.
- [ ] Overall assigned-host coverage: Skylark and Artemisia measured; six hosts pending in ../../CPU_COVERAGE.csv. This cannot be completed from an Artemisia run alone.

Signal accuracy: 99.995% (5 errors/100,000). Both controls: 50.000%. See report.md for run variation and all metrics. This is a synthetic synchronized known-bit experiment; standalone calibration accuracy is separate from held-out accuracy.

Collection: bash tools/run_ff.sh 0 1 results/artemisia/experiment-run; successful artifacts moved into results/artemisia while preserving existing benchmark files. SHA-256 manifest paths updated to the final location without changing digests.
Verification: python3 tools/verify_evidence.py results/artemisia
Graphs: python3 tools/plot_ff.py results/artemisia
