| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 89.70% ± 1.43% | 1,664,163.8 | 1,492,688.4 | 19.03% | 1.58% | 0.601 |
| unrelated_line | 50.00% ± 0.00% | 1,870,559.0 | 935,279.5 | 0.00% | 100.00% | 0.535 |
| no_victim | 50.00% ± 0.00% | 2,585,253.8 | 1,292,626.9 | 0.00% | 100.00% | 0.387 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 360 | 432 | 0.568319 | 945775.535 |
| unrelated_line | 432 | 432 | 0.000000 | 0.000 |
| no_victim | 432 | 456 | 0.000000 | 0.000 |

Setup through frozen calibration: 0.031476 s (calibration collection alone: 0.012094 s).
Shared mapping: 16,384 bytes. Parent peak RSS: 1,536 KiB; victim peak RSS: 0 KiB.
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
