# Upgrade experiment checklist

Collected 2026-10-15 on upgrade.ece.ncsu.edu (Intel Core i7-8700 CPU @ 3.20GHz), following Skylark, Artemisia, Sunbird, Charnwood, Crux and Ookay. Attacker CPU 0/core 0 and victim CPU 1/core 1 share socket 0. Inherited affinity permits CPUs 0,1,6–11.

- [x] Calibration: calibration.txt and bench_calibration.csv retain 20,000 standalone observations; diagnostic threshold 151 TSC ticks, high predicts access, 99.98% calibration accuracy. Independently collected 20,000 victim-matched observations (10,000/class) in calibration.csv; threshold.txt freezes ticks > 155 as access, with 98.525% calibration accuracy.
- [x] Repeated trials: trials.csv and summary.csv retain ten runs of 10,000 balanced observations per condition, 300,000 held-out observations total.
- [x] CPU/configuration: hostname.txt, lscpu.txt, topology.txt, allowed_cpus.txt, placement.txt, kernel.txt, compiler.txt and metadata.txt. Disassembly inspected for CLFLUSH/MFENCE and RDTSCP/LFENCE.
- [x] Controls: unrelated_line and no_victim are near 50%; exact results are in report.md. Different-line cache-set independence remains unverified; no-victim omits the handshake.
- [x] Additional measurements: report.md and resources.csv preserve errors, throughput, probe and amortized trial latency, setup time, mapping bytes and per-process peak RSS.
- [x] Reproducibility: source.c, ff_common.h, ff_experiment, bench_source.c, ff_bench, sha256.txt, raw CSVs, recorded seeds, exit_status.txt = 0 and passing verification.txt. tools/ snapshots collection and analysis scripts; python-requirements.txt records local plotting dependencies.
- [x] Graphs: figures/ contains accuracy_by_condition, errors_and_throughput, timing_distributions and setup_and_memory, each in PNG and PDF.
- [ ] Overall assigned-host coverage: seven hosts measured; Thunderbird remains pending in ../../CPU_COVERAGE.csv. Upgrade's local checklist is complete.

Signal held-out accuracy was 82.301% ± 5.78% run SD, ranging from 78.95% to 97.08%. Calibration-to-test variation is retained without test-based retuning. These results concern a synthetic synchronized known-bit signal, not unknown-secret recovery. Fixed-order drift and unverified cache-set independence remain limitations.

sc/ff_common.h already fits both threshold and polarity from fresh calibration; sc/ff_experiment.c freezes that rule before tests and applies it to every prediction. This run fitted threshold_tsc_ticks=155 and low_is_one=0. Verification independently checked minimum calibration error and all held-out predictions. No source algorithm change or hardcoded reference-host threshold was needed. The standalone 151-tick threshold is diagnostic only. Future retuning requires fresh calibration and held-out data.

Collection (repository root):
```bash
PATH="$PWD/.venv/bin:$PATH" bash tools/run_calibrated.sh 0 1 results/upgrade
```
The empty results/upgrade directory was removed before collection because the runner requires a new directory. System Python lacked NumPy/Matplotlib, and python3-venv lacked ensurepip. A repository-local .venv was bootstrapped with the existing ~/get-pip.py and populated with plotting dependencies; system Python was unchanged.

Verification: python3 tools/verify_evidence.py results/upgrade
Future full run: PATH="$PWD/.venv/bin:$PATH" bash tools/run_calibrated.sh 0 1 results/upgrade/NEW_RUN
