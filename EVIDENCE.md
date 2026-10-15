# Flush+Flush evidence

Six of seven checklist items have evidence for Skylark, Artemisia, Sunbird and Charnwood. Coverage of all eight assigned hosts is pending; portability is implemented, but portability does not establish measurements on unvisited hosts. See CPU_COVERAGE.csv.

## Checklist and artifact map

| Item | Status | Evidence |
|---|---|---|
| Calibration | Verified for Skylark | results/checklist-skylark-20261015/calibration.csv: 10,000 observations per class; threshold.txt; source.c freezes and fsyncs rule before collecting tests |
| Repeated trials | Verified for Skylark | trials.csv: three conditions, ten runs each, 10,000 fresh measurements per run, 5,000 labels of each class; summary.csv |
| CPU/configuration | Verified for Skylark | lscpu.txt, topology.txt, placement.txt, allowed_cpus.txt, compiler.txt, metadata.txt, source.c, ff_common.h, disassembly.txt |
| Controls | Verified with limitation | no_victim and unrelated_line; unrelated line is on a different page, but cache-set relationship is not verified |
| Additional measurements | Verified for Skylark | report.md and resources.csv: amortized trial latency, probe p50/p95, setup cost, mapping bytes and process peak RSS |
| Reproducibility | Verified for Skylark | original/, sc/, tools/, raw CSVs, source/header/executable snapshots and SHA-256; verification.txt |
| Assigned CPU coverage | Pending | CPU_COVERAGE.csv; four hosts still need local runs or actual unsupported/unavailable evidence |

Paths named without a directory in the table refer to results/checklist-skylark-20261015.

## Result and interpretation

On skylark.ece.ncsu.edu (AMD EPYC 7532), attacker logical CPU 0/core 0 and victim logical CPU 1/core 1 were on socket 0. The captured inherited affinity allowed 0–31. The build used cc -std=c11 -O2 -Wall -Wextra -Werror; compiler version is captured. The experiment uses two owned processes and a MAP_SHARED|MAP_ANONYMOUS mapping inherited through fork, ensuring the same physical target line. It does not use OpenSSL or attempt key recovery.

Each trial first flushes the target and waits for completion. The synthetic victim reads the target for known bit 1 and skips it for bit 0, acknowledges completion on a separate page, and the attacker times CLFLUSH+MFENCE using RDTSCP/LFENCE. No reload is performed inside the timed probe. TSC_AUX checks detect migration during an observation. Timestamp units are TSC ticks, not necessarily core cycles. Trial throughput uses CLOCK_MONOTONIC_RAW, including resets and synchronization but excluding CSV output and decoding.

After 1,000 discarded warmups, calibration interleaves 10,000 observations/class. The classifier considers both threshold directions and minimizes calibration error. This run froze `access = ticks > 408`, with calibration accuracy 93.60%. threshold.txt is flushed and fsynced before test collection. Fresh balanced label permutations and measurements are collected for each test run; calibration samples are not included in test accuracy. The recorded random seed documents input generation. The audit validates stored data consistency, not an independent attestation of collection time.

| Condition | Held-out accuracy, mean ± run SD | Trials/s | Mean trial latency |
|---|---|---|---|
| signal | 77.25% ± 2.82% | 1,626,979.2 | 0.615 microseconds |
| unrelated_line | 50.13% ± 0.21% | 1,911,135.5 | 0.523 microseconds |
| no_victim | 50.03% ± 0.18% | 2,685,373.0 | 0.372 microseconds |

Signal FPR was 45.49%, FNR 0.00%. Setup through frozen calibration took 0.032593 seconds; shared mapping was 16,384 bytes; parent peak RSS was 1,536 KiB and victim peak RSS 684 KiB. Victim RSS is the maximum of its live /proc/PID/status VmHWM and wait4 ru_maxrss because the older run reported zero using wait4 alone. Per-process RSS includes shared pages and must not be summed as unique physical memory.

This demonstrates a controlled, synchronized Flush+Flush signal above the two controls. It does not establish asynchronous AES key recovery. Error rates remain material. Conditions execute in fixed order, timestamps are temporally correlated, and drift is possible. An earlier retained run (results/poc-01, outside the portable bundle) used the opposite polarity and achieved 89.70%; this variation reinforces the need to freeze calibration per run and report controls. Do not copy 408 or the legacy histogram threshold 564 to other experiments.

The unrelated_line condition reads a separate line/page, not a verified different cache set. The no_victim condition has no handshake, so its throughput is not directly comparable. Mean trial latency is amortized, not per-trial p95 end-to-end latency. Probe p50/p95 and empirical information are in report.md; empirical information is classifier-specific and may be biased above zero by finite-sample noise. Correct bits/s includes chance successes and is not leakage capacity.

## Why the AES and spy_ff outputs were near zero

Each AES CSV cell counts the number of 10,000 post-encryption flush timings crossing its threshold for one cache line and one plaintext-byte setting. spy_ff prints the same kind of threshold count for consecutive batches on a single offset, but does not trigger encryption. Counts around 0–6 indicate almost no measurements crossing the threshold; they do not show a useful plaintext-dependent pattern. At inspection the AES executable linked system libcrypto.so.3 while mmap monitored local libcrypto.so.1.0.0. The Makefile was corrected to compile and link the local OpenSSL 1.0.2u headers/library with an explicit runtime path. Compilation and ldd were verified; AES leakage was not measured as part of this checklist run.

The standalone histogram threshold cannot be assumed valid across hosts or different probe/placement protocols. This portable synthetic experiment calibrates its own exact protocol and checks both polarities.

## Reproduce and move to other machines

Requirements: Linux x86-64, a C compiler with x86 intrinsics, Python 3 standard library, bash, coreutils (timeout, sha256sum), binutils (objdump), util-linux (lscpu, taskset), and two permitted logical CPUs. No OpenSSL, Python packages, network downloads, or absolute installation path is required. 32-bit x86 and non-x86 targets are explicitly unsupported by this harness. CPU feature checks cover TSC, CLFLUSH, SSE2 and RDTSCP; failures retain diagnostics rather than substituting another technique.

Unpack flush-flush-evidence.tar.gz in your chosen directory on each host, then:

```bash
cd flush-flush-evidence
hostname
lscpu -e=CPU,CORE,SOCKET,ONLINE
taskset -pc $$
# Replace 0 and 1 with two permitted, different logical CPU IDs.
bash tools/run_local.sh 0 1
# The runner prints the report. Use the newly created results directory below:
python3 tools/verify_evidence.py results/HOST-TIMESTAMP-PID
```

Do not wrap the runner in a single-CPU taskset mask. Prefer consistent topology across hosts, recording deviations. Each invocation uses a new directory and separately frozen threshold. The Skylark collection command was:

```bash
bash tools/run_ff.sh 0 1 results/checklist-skylark-20261015
python3 tools/verify_evidence.py results/checklist-skylark-20261015
```

Preserve failed directories, stderr and exit status. An unsupported result needs the actual feature/instruction failure. An unavailable host needs the actual access/allocation error and date; a pending run is not an unsupported result. Collect teammates' result directories and update CPU_COVERAGE.csv with the evidence path/status. Do not mark coverage complete until every assigned host has measured results or documented unavailable/unsupported evidence. No teammate messages were sent.

## Artemisia — 2026-10-15

Completed the same protocol as Skylark on artemisia.ece.ncsu.edu (Intel Xeon Gold 5420+), attacker CPU 0 and victim CPU 1, separate physical cores on socket 0. All artifacts and four figures in PNG/PDF are in results/artemisia. See results/artemisia/checklist.md, report.md, and verification.txt.

The user's standalone calibration (524 ticks, low predicts 1) is preserved in calibration.txt and bench_calibration.csv. The experiment independently froze ticks <= 248 as bit 1, with 99.995% calibration accuracy. Across 10 fresh runs per condition, signal held-out accuracy was 99.995% (5 errors in 100,000 trials); both controls were exactly 50%. This supports a controlled synchronized signal on this host. The different-line control's cache-set relationship remains unverified. Overall coverage is two of eight hosts, with six pending.

## Sunbird — 2026-10-15

Completed the same protocol on sunbird.ece.ncsu.edu (Intel Xeon E5-2680 v3), with attacker CPU 0 and victim CPU 2 on separate cores of socket 0. CPU 1 was excluded by inherited affinity. All raw observations, source/executable snapshots, calibration, metrics, verification and four PNG/PDF figure pairs are in results/sunbird; see checklist.md and report.md there.

The standalone calibration selected 280 TSC ticks (low predicts access). The experiment independently froze ticks <= 204, with 100% calibration accuracy. Held-out signal accuracy was 98.13% ± 5.90% run SD; both controls were 50%. Signal run 1 achieved 81.33%, while the remaining nine achieved 99.99–100%; this variability is preserved without retuning. Verification passed for 20,000 calibration and 300,000 held-out observations. Coverage is now three of eight hosts; five remain pending. The existing limitations about synchronization, cache-set independence and fixed-order drift apply.

## Charnwood — 2026-10-15

Completed the same protocol on charnwood.ece.ncsu.edu (Intel Core i7-6700), attacker CPU 0 and victim CPU 1, separate physical cores on socket 0. Calibration, raw trials, source/executable snapshots, metrics, verification, checklist and four PNG/PDF figure pairs are in results/charnwood.

The standalone calibration selected 156 TSC ticks (high predicts access), with 99.34% calibration accuracy. The experiment independently froze ticks > 134, with 99.97% calibration accuracy. The existing classifier automatically adjusted threshold and polarity before tests; no reference-host constants were reused. Held-out signal accuracy was 99.55% ± 0.63% run SD (452 errors/100,000); unrelated-line and no-victim controls achieved 49.98% and 50.00%. Verification passed for 20,000 calibration and 300,000 held-out observations. Coverage is four of eight hosts, with four pending. Synchronization, unverified cache-set independence and fixed-order drift remain limitations.

Reproduce with a fresh output directory: bash tools/run_calibrated.sh 0 1 results/charnwood/NEW_RUN.
