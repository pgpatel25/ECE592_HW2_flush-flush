# Ookay experiment checklist

Collected 2026-10-15 on ookay.ece.ncsu.edu (Intel Core i7-7700 CPU @ 3.60GHz), following Skylark, Artemisia, Sunbird, Charnwood and Crux. Attacker CPU 0/core 0 and victim CPU 1/core 1 share socket 0; inherited affinity allows CPUs 0–7.

- [x] Calibration: calibration.txt and bench_calibration.csv preserve 20,000 standalone observations (threshold 150 TSC ticks, high predicts access; calibration accuracy 99.76%). Independently collected 20,000 victim-matched observations in calibration.csv (10,000/class). Experiment froze ticks > 150 as bit 1 before tests; calibration accuracy 99.57%.
- [x] Repeated trials: trials.csv and summary.csv contain ten runs of 10,000 balanced trials per condition, 300,000 held-out observations total.
- [x] CPU/configuration: hostname.txt, lscpu.txt, topology.txt, allowed_cpus.txt, placement.txt, kernel.txt, compiler.txt and metadata.txt. Disassembly inspected for CLFLUSH/MFENCE and RDTSCP/LFENCE.
- [x] Controls: unrelated_line and no_victim achieved approximately 50.02% and 49.99%. Different-line cache-set independence remains unverified; no-victim omits the handshake.
- [x] Additional measurements: report.md and resources.csv contain errors, throughput, latency, setup, mapping size and per-process peak RSS.
- [x] Reproducibility: source.c, ff_common.h, ff_experiment, bench_source.c, ff_bench, sha256.txt, raw CSVs, recorded seeds, exit_status.txt = 0 and passing verification.txt. python-requirements.txt records plotting dependency versions; tools/ snapshots the collection and analysis scripts.
- [x] Graphs: figures/ contains accuracy_by_condition, errors_and_throughput, timing_distributions and setup_and_memory, each in PNG and PDF.
- [ ] Overall assigned-host coverage: Ookay, Crux, Charnwood, Sunbird, Skylark and Artemisia measured; Thunderbird and Upgrade remain pending in ../../CPU_COVERAGE.csv. Ookay's local checklist is complete; overall coverage is separate.

Signal held-out accuracy: 92.995% ± 3.96% run SD; 7,005 errors/100,000. FPR: 14.008%; FNR: 0.002%. Individual signal runs ranged from 88.49% to 99.79%. This variation is retained without test-based retuning. Controls remain near balanced chance. These measurements support a synthetic synchronized known-bit signal, not unknown-secret recovery.

The shared classifier in sc/ff_common.h automatically fits threshold and polarity to fresh machine-specific calibration. Ookay fitted threshold_tsc_ticks=150 and low_is_one=0, applied before every held-out prediction by sc/ff_experiment.c. No reference-host constant is reused and no algorithm change was necessary. The standalone threshold also happened to be 150 but is diagnostic only; the experiment independently fitted its own rule. A future retuning requires new calibration and fresh held-out data.

Collection and figures (repository root):
```bash
PATH="$PWD/.venv/bin:$PATH" bash tools/run_calibrated.sh 0 1 results/ookay
```
The existing empty results/ookay directory was removed before collection because the runner requires a new directory. Initial dependency preflight failed before measurements because Matplotlib was absent. A local .venv was created and populated with NumPy/Matplotlib without changing system Python; the successful full run then produced these artifacts.

Verification: python3 tools/verify_evidence.py results/ookay
Future full run (fresh directory): PATH="$PWD/.venv/bin:$PATH" bash tools/run_calibrated.sh 0 1 results/ookay/NEW_RUN
