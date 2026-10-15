# Local Flush+Flush experiment

The previous `sc/ff_bench.c` is saved as `sc/ff_bench.c.before-update`. Both updated programs include `sc/ff_common.h`, which supplies the missing `rdtsc`, `flush`, `maccess`, and `flushandreload` helpers using `<x86intrin.h>`. Like the supplied `ff.c`, they time a flush; there is no reload inside the timed interval. The shared helper adds completion/ordering fences and checks TSC_AUX. Both programs also share threshold selection and prediction code. The original hardcoded 172–200 window and the previous 97-cycle histogram suggestion are not reused.

This harness uses only a synthetic victim process it creates, known balanced bits, and an anonymous shared mapping inherited through fork. The target is consequently the same physical cache line in both processes. It does not inspect another application's secrets. Use only resources allowed by Homework 1; those rules were not supplied here. An idle CPU is not evidence of an allocation. Confirm permitted CPU IDs before running.

## Build and collect

To compile both programs and run the simple calibration on an authorized CPU:

```bash
cd /home/pgpatel/ECE592_HW2_flush-flush
cc -std=c11 -O2 -Wall -Wextra sc/ff_bench.c -o sc/ff_bench
cc -std=c11 -O2 -Wall -Wextra sc/ff_experiment.c -o sc/ff_experiment
mkdir results/bench-local-01
cd results/bench-local-01
taskset -c 0 ../../sc/ff_bench | tee calibration.txt
cd ../..
```

CPU 0 is your stated calibration placement; use it only if permitted. Use a new output directory each time. `bench_calibration.csv` retains all 20,000 calibration observations, and the terminal output gives the threshold, its direction, and calibration accuracy. This single-process benchmark is a diagnostic; its accuracy is not a held-out experiment result. The experiment uses the same timing function but independently freezes a threshold with its actual victim and core placement. Do not copy the bench threshold into the experiment.

From this repository:

```bash
lscpu -e=CPU,CORE,SOCKET,ONLINE
taskset -pc $$
# Replace these with two permitted logical CPU IDs:
ATTACKER_CPU=0
VICTIM_CPU=1
./tools/run_ff.sh "$ATTACKER_CPU" "$VICTIM_CPU" results/local-ff-01
```

The CPU values above are examples, not an allocation. Do not launch the runner under `taskset -c 0`: it checks both IDs against its inherited CPU mask. Choose different logical CPUs; record whether they share a physical core, socket, and cache. Reuse the exact same CPU pair and input generation protocol for comparisons. This harness does not support both processes on one logical CPU because its spin handshake would starve the victim.

The runner builds a source and shared-header snapshots with strict compiler warnings and saves topology, allowed CPU mask, kernel, compiler, executable hash, disassembly, and exit status. The program pins both processes, checks required CPUID features on both CPUs, executes the primitives, checks monotonic timestamps and TSC_AUX equality within each observation, and aborts on invalid observations. Inspect disassembly for the intended CLFLUSH/MFENCE and RDTSCP/LFENCE sequence. Timestamp units are **TSC ticks**, not necessarily core cycles. These checks do not establish TSC frequency or cross-core synchronization; elapsed throughput uses CLOCK_MONOTONIC_RAW instead. No cross-core timestamp subtraction is used.

## Measurements and outputs

After 1,000 discarded warmups, calibration randomly interleaves exactly 10,000 observations of each class using the same victim placement as the signal trials. The classifier minimizes calibration errors over thresholds and both polarities, with rule `ticks <= threshold`. It flushes and syncs `threshold.txt` to disk before test collection. Calibration samples never enter reported test accuracy.

Each of the three conditions has 10 fresh runs of 10,000 observations, with 5,000 zero and 5,000 one labels independently shuffled per run. The PRNG seed is recorded; fresh means new measurements and shuffles, not cryptographically secret inputs.

- `signal`: victim reads the shared target for bit 1 and skips it for bit 0.
- `unrelated_line`: identical handshake, but bit 1 reads a different page and cache line. Its cache-set relationship is **unverified**, so this is a line control, not a verified unrelated-set control.
- `no_victim`: victim process is terminated; labels remain balanced and the attacker flushes/times its target without a handshake. Its different synchronization overhead means its throughput is not directly comparable to the signal condition.

Every trial starts with CLFLUSH+MFENCE to reset the target. Signal and unrelated-line conditions wait for victim completion before timing CLFLUSH+MFENCE on the target. Thus this measures a synchronized, controlled channel, not asynchronous recovery from an unknown victim. Label delivery and acknowledgements occupy separate pages from the observed line. Controls help expose timing effects from synchronization rather than target accesses.

`calibration.csv` and `trials.csv` retain unbinned timestamps and known labels. `summary.csv` includes confusion counts, accuracy, trials/s, and correct bits/s per run. `report.md` summarizes run variation and rates weighted by elapsed time. Timing includes resets and handshakes but excludes shuffling and CSV output; it is not end-to-end file-generation throughput. Correct bits/s counts chance successes too; it is **not** a leakage-capacity estimate. Chance-level accuracy establishes no leakage. Compare held-out signal accuracy against both controls and run variation before drawing a conclusion.

Conditions run in fixed order. This initial harness does not eliminate drift or all branch/prefetch effects; repeat full experiments and examine raw distributions rather than interpreting a tiny excess over 50% as proof. Do not retune thresholds using test results. Any retuning requires a new, separately identified calibration and fresh dataset.

## Coverage and limitations

CLFLUSH requires shared memory, which is constructed explicitly here. Eviction sets are not applicable to this Flush+Flush implementation. This does **not** validate eviction-based techniques. Other primitives are not implemented; unavailable CPUs and unsupported instruction executions must be reported as such, not replaced with another technique. Run separately on each authorized CPU pair and preserve failed runs and their error/status files. A partial run is not a completed dataset. CPU support flags alone cannot guarantee OS/hypervisor permission; execution failures are retained by the runner.

Measurements and checked artifact maps are available in EVIDENCE.md, results/checklist-skylark-20261015, results/artemisia, and results/sunbird. The portable tools/run_local.sh wrapper supports collection on other Linux x86-64 hosts. Use CPUs permitted by your allocation; five other hosts are still pending.

Timing reference: Intel's instruction reference documents RDTSCP ordering and use of MFENCE/LFENCE: https://cdrdv2-public.intel.com/774492/325383-sdm-vol-2abcd.pdf . The chosen observable includes MFENCE completion cost, so it must be calibrated independently from the original benchmark.

## Assignment metrics

The runner's `report.md` now includes the following, calculated from fresh held-out trials:

| Metric | Definition |
|---|---|
| Accuracy | `(TP + TN) / trials`, with mean and standard deviation across runs |
| False-positive rate | `FP / (FP + TN)`: predicts target access when known bit is 0 |
| False-negative rate | `FN / (FN + TP)`: misses target access when known bit is 1 |
| Trial rate | Total trials / total measured batch seconds |
| Correct bits/s | Total correct predictions / total measured batch seconds; includes chance successes |
| Empirical information rate | Empirical mutual information between known and predicted bits, times trial rate; classifier-specific and not channel capacity |
| Mean trial latency | Total measured batch seconds / total trials, reported in microseconds |
| Probe latency | p50 and p95 fenced flush durations from raw trials, in TSC ticks |
| Setup cost | Program entry through warmup, calibration, threshold selection, and threshold-file flush/sync; excludes compilation and runner metadata capture |
| Memory use | Parent and victim peak RSS separately (Linux KiB), plus shared mapping size in bytes |

`resources.csv` records setup time, calibration collection time, shared mapping bytes, parent peak RSS, and victim peak RSS. Parent RSS covers the experiment through all conditions; victim RSS covers its lifetime until termination before the no-victim condition. Summing RSS double-counts shared pages and is not unique physical memory use. The measurement buffers and runtime overhead are included in RSS.

Mean trial latency is an amortized batch measurement. It is not a p95 end-to-end latency; only probe TSC durations have percentiles. Setup time is one observation per program invocation; repeat full experiments if setup variance is required. Setup includes calibration CSV output, while calibration collection time excludes its output. Throughput batches exclude prediction/counting and CSV output, so rates describe probe collection, not full decoding or application throughput.

An empirical information estimate can be positive from finite-sample noise. Report controls and repeated-run accuracy alongside it; a chance-level PoC does not demonstrate leakage even if the correct-bits/s number looks large. This is a synthetic, explicitly synchronized known-bit PoC, not evidence of extracting unknown application secrets.

## Complete calibration and figure workflow

Run `bash tools/run_calibrated.sh ATTACKER_CPU VICTIM_CPU NEW_OUTPUT_DIRECTORY` to collect the independently calibrated experiment, standalone benchmark, verification and PNG/PDF figures together. Python NumPy and Matplotlib are required for figures. The classifier recalibrates threshold and polarity on every invocation before held-out collection; standalone benchmark thresholds are diagnostic only. On Sunbird, use permitted same-socket CPUs 0 and 2 (CPU 1 is excluded by inherited affinity). Completed Sunbird artifacts and checklist are in results/sunbird. Use a fresh directory for subsequent runs.
