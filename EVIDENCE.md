# Flush+Flush evidence

All seven checklist items now have evidence across Skylark, Artemisia, Sunbird, Charnwood, Crux, Ookay, Upgrade and Charlotte. Charlotte replaces Thunderbird per the user’s updated assignment. All eight assigned hosts are measured; see CPU_COVERAGE.csv. Measurement completion does not imply reliable decoding on every host. Historical coverage counts below describe the state at each earlier run.

## Checklist and artifact map

| Item | Status | Evidence |
|---|---|---|
| Calibration | Verified for Skylark | results/checklist-skylark-20261015/calibration.csv: 10,000 observations per class; threshold.txt; source.c freezes and fsyncs rule before collecting tests |
| Repeated trials | Verified for Skylark | trials.csv: three conditions, ten runs each, 10,000 fresh measurements per run, 5,000 labels of each class; summary.csv |
| CPU/configuration | Verified for Skylark | lscpu.txt, topology.txt, placement.txt, allowed_cpus.txt, compiler.txt, metadata.txt, source.c, ff_common.h, disassembly.txt |
| Controls | Verified with limitation | no_victim and unrelated_line; unrelated line is on a different page, but cache-set relationship is not verified |
| Additional measurements | Verified for Skylark | report.md and resources.csv: amortized trial latency, probe p50/p95, setup cost, mapping bytes and process peak RSS |
| Reproducibility | Verified for Skylark | original/, sc/, tools/, raw CSVs, source/header/executable snapshots and SHA-256; verification.txt |
| Assigned CPU coverage | Complete | CPU_COVERAGE.csv; Charlotte replaces Thunderbird; all eight hosts measured; results/charlotte/checklist.md |

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

## Crux — 2026-10-15

Completed the reference protocol on crux.ece.ncsu.edu (Intel Core i7-9700), attacker CPU 0 and victim CPU 1, separate physical cores on socket 0. Calibration, raw trials, source/executable snapshots, metrics, verification, checklist and four PNG/PDF figure pairs are in results/crux.

Standalone calibration selected 111 TSC ticks (high predicts access), with 99.98% calibration accuracy. The experiment independently froze ticks > 120, with 93.14% calibration accuracy. The shared code fitted and applied Crux's threshold and polarity before held-out collection; no hardcoded reference threshold or algorithm change was needed. Verification passed for 20,000 calibration and 300,000 held-out observations, plus snapshot hashes and resource measurements.

Held-out signal accuracy was 61.525% ± 11.10% run SD (38,475 errors/100,000); unrelated-line and no-victim controls achieved 49.926% and 50.059%. Signal run 1 achieved 93.12%, followed by 57.68–58.28% in runs 2–10. This calibration-to-test variation is retained without test-based retuning. Results do not establish reliable decoding on Crux. Synchronization, unverified cache-set independence and fixed-order drift remain limitations. Coverage is five of eight hosts; Thunderbird, Ookay and Upgrade remain pending.

Reproduce with a fresh directory: bash tools/run_calibrated.sh 0 1 results/crux/NEW_RUN.

## Ookay — 2026-10-15

Completed the reference protocol on ookay.ece.ncsu.edu (Intel Core i7-7700), attacker CPU 0 and victim CPU 1, separate physical cores on socket 0. Calibration, raw trials, source/executable snapshots, metrics, verification, checklist and four PNG/PDF figure pairs are in results/ookay.

The standalone calibration selected 150 TSC ticks (high predicts access), with 99.76% calibration accuracy. The experiment independently froze ticks > 150, with 99.57% calibration accuracy. The existing shared classifier fitted and applied Ookay's threshold and polarity before held-out collection; no algorithm change or reference-host constant was needed. Verification passed for 20,000 calibration and 300,000 held-out observations, snapshot hashes and resource measurements.

Held-out signal accuracy was 92.995% ± 3.96% run SD (7,005 errors/100,000); unrelated-line and no-victim controls were approximately 50.02% and 49.99%. Signal runs ranged from 88.49% to 99.79%, without test-based retuning. Synchronization, unverified cache-set independence and fixed-order drift remain limitations. Coverage is six of eight hosts; Thunderbird and Upgrade remain pending.

Reproduce with a fresh directory: PATH="$PWD/.venv/bin:$PATH" bash tools/run_calibrated.sh 0 1 results/ookay/NEW_RUN. Plotting package versions are preserved in results/ookay/python-requirements.txt.

## Upgrade — 2026-10-15

Completed the reference protocol on upgrade.ece.ncsu.edu (Intel Core i7-8700), attacker CPU 0 and victim CPU 1 on separate physical cores of socket 0. All calibration, raw trials, metrics, source/executable snapshots, verification, checklist and four PNG/PDF figure pairs are in results/upgrade.

Standalone calibration selected 151 TSC ticks (high predicts access), with 99.98% calibration accuracy. The experiment independently froze ticks > 155, with 98.525% calibration accuracy. The existing shared classifier fitted and applied Upgrade's threshold and polarity before tests. No source algorithm change or reference-host constant was required. Verification passed for 20,000 calibration observations, 300,000 held-out observations, minimum-error calibration, predictions, confusion counts, snapshot hashes and resources.

Signal accuracy was 82.301% ± 5.78% run SD, ranging from 78.95% to 97.08%; controls remained near 50%. Calibration-to-test variation is preserved without retuning. Synchronization, fixed-order drift and unverified cache-set independence remain limitations. Coverage is seven of eight hosts; Thunderbird remains pending.

Reproduce in a fresh directory: PATH="$PWD/.venv/bin:$PATH" bash tools/run_calibrated.sh 0 1 results/upgrade/NEW_RUN. Plotting dependencies are recorded in results/upgrade/python-requirements.txt.

## Charlotte — 2026-10-15

Charlotte replaces Thunderbird per the user’s updated assignment. Completed the reference protocol on charlotte.ece.ncsu.edu (Intel Core i7-6700), attacker CPU 0 and victim CPU 1 on separate physical cores of socket 0; inherited affinity permits CPUs 0–7. All calibration, raw trials, metrics, source/executable and tool snapshots, verification, checklist and four PNG/PDF figure pairs are in results/charlotte.

Fresh victim-matched calibration froze ticks > 424 as access, with 87.185% calibration accuracy. The independent standalone diagnostic selected ticks > 214 (87.50%). The existing shared code fitted and applied Charlotte’s threshold and polarity before tests; no algorithm edit or copied host constant was needed. Verification passed for 20,000 calibration observations and 300,000 held-out observations, predictions, confusion counts, snapshot hashes and resources.

Held-out signal accuracy was 50.006% (49,994 errors/100,000); unrelated_line was 49.997% and no_victim 50.002%. Calibration did not transfer to held-out timings. These results do not demonstrate reliable decoding or leakage on Charlotte. The frozen rule is retained without test-based retuning. Fixed-order drift, synchronized known-bit inputs and unverified cache-set independence remain limitations. Overall assigned-host coverage is now eight of eight, with Charlotte substituted for Thunderbird.

The machine lacked a native compiler, binutils and plotting packages. Local extracted Ubuntu compiler/binutils packages and a repository .venv enabled the unchanged collection workflow; see results/charlotte/build_environment.txt and python-requirements.txt.
