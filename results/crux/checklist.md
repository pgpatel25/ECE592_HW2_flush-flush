# Crux experiment checklist

Collected 2026-10-15 on crux.ece.ncsu.edu (Intel Core i7-9700 CPU @ 3.00GHz), following Skylark, Artemisia, Sunbird and Charnwood. Attacker CPU 0/core 0 and victim CPU 1/core 1 share socket 0; inherited affinity allows CPUs 0–7.

- [x] Calibration: calibration.txt and bench_calibration.csv preserve 20,000 standalone observations (threshold 111 TSC ticks, high predicts access; calibration accuracy 99.98%). Independently collected 20,000 victim-matched observations in calibration.csv (10,000/class). Experiment froze ticks > 120 as bit 1 before tests; calibration accuracy 93.14%.
- [x] Repeated trials: trials.csv and summary.csv contain ten runs of 10,000 balanced trials per condition, 300,000 held-out observations total.
- [x] CPU/configuration: hostname.txt, lscpu.txt, topology.txt, allowed_cpus.txt, placement.txt, kernel.txt, compiler.txt and metadata.txt. Disassembly inspected for CLFLUSH/MFENCE and RDTSCP/LFENCE.
- [x] Controls: unrelated_line achieved 49.926% and no_victim achieved 50.059%. Different-line cache-set independence remains unverified; no-victim omits the handshake.
- [x] Additional measurements: report.md and resources.csv contain errors, throughput, latency, setup, mapping size and per-process peak RSS.
- [x] Reproducibility: source.c, ff_common.h, ff_experiment, bench_source.c, ff_bench, sha256.txt, raw CSVs, recorded seeds, exit_status.txt = 0 and passing verification.txt.
- [x] Graphs: figures/ contains accuracy_by_condition, errors_and_throughput, timing_distributions and setup_and_memory, each in PNG and PDF.
- [ ] Overall assigned-host coverage: Crux, Charnwood, Sunbird, Skylark and Artemisia measured; Thunderbird, Ookay and Upgrade remain pending in ../../CPU_COVERAGE.csv. Crux's local checklist is complete; overall coverage is separate.

Signal held-out accuracy: 61.525% ± 11.10% run SD; 38,475 errors/100,000. FPR: 76.768%; FNR: 0.182%. First signal run: 93.12%; subsequent runs: 57.68–58.28%. This variation is retained without test-based retuning. Controls remain near balanced chance. These measurements concern a synthetic synchronized known-bit signal, not unknown-secret recovery, and do not establish reliable decoding on Crux.

The shared classifier in sc/ff_common.h automatically fits threshold and polarity to fresh machine-specific calibration. Crux fitted threshold_tsc_ticks=120 and low_is_one=0, applied before every held-out prediction. No reference-host constant is reused and no algorithm change was necessary. The standalone threshold 111 is diagnostic only and is not used for experiment predictions. Calibration-to-test variation is visible in the raw observations and figures; a future retuning must use a new calibration and fresh dataset.

Collection and figures: bash tools/run_calibrated.sh 0 1 results/crux
Verification: python3 tools/verify_evidence.py results/crux
Future full run (fresh directory): bash tools/run_calibrated.sh 0 1 results/crux/NEW_RUN
