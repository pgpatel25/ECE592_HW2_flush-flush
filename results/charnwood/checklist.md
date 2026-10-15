# Charnwood experiment checklist

Collected 2026-10-15 on charnwood.ece.ncsu.edu (Intel Core i7-6700 CPU @ 3.40GHz), following the Artemisia, Skylark and Sunbird protocol. Attacker CPU 0/core 0 and victim CPU 1/core 1 share socket 0; inherited affinity allows CPUs 0–7.

- [x] Calibration: calibration.txt and bench_calibration.csv preserve 20,000 standalone observations (threshold 156 TSC ticks, high predicts 1; calibration accuracy 99.34%). Independently collected 20,000 victim-matched observations in calibration.csv (10,000/class). Experiment froze ticks > 134 as bit 1 before tests; calibration accuracy 99.97%.
- [x] Repeated trials: trials.csv and summary.csv contain ten runs of 10,000 balanced trials per condition, 300,000 held-out observations total.
- [x] CPU/configuration: hostname.txt, lscpu.txt, topology.txt, allowed_cpus.txt, placement.txt, kernel.txt, compiler.txt and metadata.txt. Inspected disassembly.txt for CLFLUSH/MFENCE and RDTSCP/LFENCE.
- [x] Controls: unrelated_line achieved 49.98% and no_victim achieved 50.00%. Different-line cache-set independence remains unverified; no-victim omits the handshake.
- [x] Additional measurements: report.md and resources.csv contain errors, throughput, latency, setup, mapping size and per-process peak RSS.
- [x] Reproducibility: source.c, ff_common.h, ff_experiment, bench_source.c, ff_bench, sha256.txt, raw CSVs, recorded seeds, exit_status.txt = 0 and passing verification.txt.
- [x] Graphs: figures/ contains accuracy_by_condition, errors_and_throughput, timing_distributions and setup_and_memory, each in PNG and PDF.
- [ ] Overall assigned-host coverage: Charnwood, Sunbird, Skylark and Artemisia measured; four hosts remain pending in ../../CPU_COVERAGE.csv. Charnwood's local checklist is complete; overall host coverage is separate.

Signal held-out accuracy: 99.548% (rounded report: 99.55%) ± 0.63% run SD; 452 errors/100,000. FPR: 0.53%; FNR: 0.38%. Both controls remain near balanced chance. These measurements support a synthetic synchronized known-bit signal, not unknown-secret recovery.

The shared classifier in sc/ff_common.h automatically fits both threshold and polarity to this machine's fresh calibration. Charnwood requires the high-timing direction (low_is_one=0), unlike Artemisia and Sunbird. The experiment applied this adjustment before all test trials; no hardcoded reference-host constant or test-based retuning was introduced. The standalone threshold 156 is diagnostic and is not used for experiment predictions. Existing calibration code needed no algorithm change.

Collection and figures: bash tools/run_calibrated.sh 0 1 results/charnwood
Verification: python3 tools/verify_evidence.py results/charnwood
Future full run (fresh directory): bash tools/run_calibrated.sh 0 1 results/charnwood/NEW_RUN
