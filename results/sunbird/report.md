| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 98.13% ± 5.90% | 2,014,320.9 | 1,976,653.1 | 0.00% | 3.74% | 0.496 |
| unrelated_line | 50.00% ± 0.00% | 2,183,851.0 | 1,091,925.5 | 0.00% | 100.00% | 0.458 |
| no_victim | 50.00% ± 0.00% | 3,891,778.2 | 1,945,889.1 | 0.00% | 100.00% | 0.257 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 288 | 304 | 0.883870 | 1780397.817 |
| unrelated_line | 292 | 296 | 0.000000 | 0.000 |
| no_victim | 292 | 296 | 0.000000 | 0.000 |

Setup through frozen calibration: 0.090611 s (calibration collection alone: 0.010668 s).
Shared mapping: 16,384 bytes. Parent peak RSS: 1,920 KiB; victim peak RSS: 688 KiB.
Peak RSS values are per process, include runtime overhead, and double-count shared pages if added.
Mean trial latency is batch elapsed time / trials, not an individually timed latency distribution.
Flush quantiles measure only the fenced probe in TSC ticks, not wall-clock microseconds.
Empirical information rate is a finite-sample classifier estimate, not channel capacity or proof of leakage;
it can be positively biased at chance and must be assessed against controls and repeated runs.

Correct bits/s = correct decisions / elapsed wall time; this includes chance guesses.
Chance accuracy is 50%. Compare signal with both controls and the variation across runs.
Above-chance calibration alone does not establish leakage. Fixed-order conditions may drift;
repeat the full experiment to assess reproducibility. Raw trials are temporally correlated;
do not treat every trial as an independent statistical replicate.
