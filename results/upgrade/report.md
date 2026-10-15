| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 82.30% ± 5.78% | 3,318,591.8 | 2,731,234.3 | 35.27% | 0.13% | 0.301 |
| unrelated_line | 50.18% ± 0.46% | 3,904,729.9 | 1,959,237.3 | 39.89% | 59.76% | 0.256 |
| no_victim | 50.05% ± 0.57% | 8,688,818.2 | 4,348,579.7 | 34.33% | 65.57% | 0.115 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 158 | 165 | 0.433751 | 1439442.528 |
| unrelated_line | 155 | 158 | 0.000009 | 36.333 |
| no_victim | 155 | 157 | 0.000001 | 6.401 |

Setup through frozen calibration: 0.019796 s (calibration collection alone: 0.006057 s).
Shared mapping: 16,384 bytes. Parent peak RSS: 2,432 KiB; victim peak RSS: 932 KiB.
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
