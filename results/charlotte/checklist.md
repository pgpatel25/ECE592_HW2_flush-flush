# Charlotte experiment checklist

Collected 2026-10-15 on charlotte.ece.ncsu.edu (Intel Core i7-6700 CPU @ 3.40GHz), following Skylark, Artemisia, Sunbird, Charnwood, Crux, Ookay and Upgrade. Charlotte replaces Thunderbird per the user's updated assignment. Attacker CPU 0/core 0 and victim CPU 1/core 1 share socket 0; inherited affinity permits CPUs 0–7.

- [x] Calibration: calibration.txt and bench_calibration.csv preserve 20,000 standalone observations; diagnostic threshold 214 TSC ticks, high predicts access, 87.50% calibration accuracy. Independently collected 20,000 victim-matched observations (10,000/class) in calibration.csv; threshold.txt freezes ticks > 424 as access, with 87.185% calibration accuracy.
- [x] Repeated trials: trials.csv and summary.csv preserve ten runs of 10,000 balanced observations per condition, 300,000 held-out observations total.
- [x] CPU/configuration: hostname.txt, lscpu.txt, topology.txt, allowed_cpus.txt, placement.txt, kernel.txt, compiler.txt and metadata.txt. Disassembly inspected for CLFLUSH/MFENCE and RDTSCP/LFENCE.
- [x] Controls: unrelated_line accuracy 49.997%; no_victim accuracy 50.002%. Different-line cache-set independence remains unverified; no-victim omits the handshake.
- [x] Additional measurements: report.md and resources.csv preserve errors, throughput, probe and amortized trial latency, setup cost, mapping bytes and per-process peak RSS.
- [x] Reproducibility: source.c, ff_common.h, ff_experiment, bench_source.c, ff_bench, sha256.txt, raw CSVs, recorded seeds, exit_status.txt = 0 and passing verification.txt. tools/ snapshots collection and analysis scripts; python-requirements.txt and build_environment.txt document local dependencies.
- [x] Graphs: figures/ contains accuracy_by_condition, errors_and_throughput, timing_distributions and setup_and_memory, each in PNG and PDF.
- [x] Overall assigned-host coverage: all eight hosts measured in ../../CPU_COVERAGE.csv; Charlotte replaces Thunderbird. Earlier reference checklists retain historical pending status.

Signal held-out accuracy was 50.006% ± 0.01075% run SD (49,994 errors/100,000), ranging from 49.99% to 50.02%. Calibration did not transfer to held-out timings. This run does not demonstrate reliable decoding or leakage on Charlotte. Calibration-to-test variation is retained without test-based retuning. These results concern a synthetic synchronized known-bit signal, not unknown-secret recovery. Fixed-order drift and unverified cache-set independence remain limitations.

sc/ff_common.h fits both threshold and polarity from fresh calibration; sc/ff_experiment.c freezes that rule before tests and applies it to every prediction. Charlotte fitted threshold_tsc_ticks=424 and low_is_one=0. Verification independently checked minimum calibration error and every held-out prediction. No source algorithm change or reference-host threshold was needed. The standalone 214-tick threshold is diagnostic only. Future retuning requires fresh calibration and held-out data.

Collection used bash tools/run_calibrated.sh 0 1 results/charlotte with the local compiler/binutils and .venv environment described in build_environment.txt. The initially empty Charlotte directory was removed because the runner requires a fresh directory. No existing measurements were removed.

Verification: python3 tools/verify_evidence.py results/charlotte
Future runs must use a fresh directory, for example results/charlotte/NEW_RUN.
