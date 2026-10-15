# Sunbird experiment checklist

Collected 2026-10-15 on sunbird.ece.ncsu.edu (Intel Xeon E5-2680 v3), following the Artemisia and Skylark protocol. Attacker CPU 0/core 0 and victim CPU 2/core 2 share socket 0. CPU 1 is outside the inherited affinity, so CPU 2 preserves the reference separate-core, same-socket placement.

- [x] Calibration: calibration.txt and bench_calibration.csv preserve 20,000 standalone observations (threshold 280, low predicts 1). Independently collected 20,000 victim-matched observations in calibration.csv. Experiment froze ticks <= 204 as bit 1 before collecting tests; calibration accuracy 100%.
- [x] Repeated trials: trials.csv and summary.csv contain ten runs of 10,000 balanced trials per condition, 300,000 total.
- [x] CPU/configuration: hostname.txt, lscpu.txt, topology.txt, allowed_cpus.txt, placement.txt, kernel.txt, compiler.txt and metadata.txt. Inspected disassembly.txt for CLFLUSH/MFENCE and RDTSCP/LFENCE.
- [x] Controls: unrelated_line and no_victim each achieved 50%. Different-line cache-set independence remains unverified; no-victim omits the handshake.
- [x] Additional measurements: report.md and resources.csv contain errors, throughput, latency, setup, mapping size and per-process peak RSS.
- [x] Reproducibility: source.c, ff_common.h, ff_experiment, bench_source.c, ff_bench, sha256.txt, raw CSVs, recorded seeds, exit_status.txt = 0 and passing verification.txt.
- [x] Graphs: figures/ contains accuracy_by_condition, errors_and_throughput, timing_distributions and setup_and_memory, each in PNG and PDF.
- [ ] Overall assigned-host coverage: Sunbird, Skylark and Artemisia measured; five hosts remain pending in ../../CPU_COVERAGE.csv. Sunbird's local checklist is complete; overall host coverage is separate.

Signal held-out accuracy: 98.13% ± 5.90% run SD (1,870 errors/100,000). First signal run: 81.33%; subsequent runs: 99.99–100%. This variation is retained; no threshold was retuned from test outcomes. Both controls: 50.00%. These measurements support a synthetic synchronized known-bit signal, not unknown-secret recovery.

The existing shared classifier automatically calibrates both threshold and polarity for this machine; no reference-host constant is copied. The standalone threshold 280 is diagnostic and is not used for experiment predictions. tools/plot_ff.py uses constrained_layout=True for this host's older Matplotlib. tools/run_calibrated.sh now reproduces standalone calibration, independently calibrated experiment, verification and figures in a fresh output directory.

Collection: bash tools/run_ff.sh 0 2 results/sunbird/experiment-run; artifacts moved into results/sunbird and manifest paths updated. Standalone benchmark built with strict warnings and run with taskset -c 0 in results/sunbird.
Verification: python3 tools/verify_evidence.py results/sunbird
Figures: python3 tools/plot_ff.py results/sunbird
Future full run: bash tools/run_calibrated.sh 0 2 results/sunbird/NEW_RUN
