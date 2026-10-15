| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 92.99% ± 3.96% | 3,513,269.3 | 3,267,164.8 | 14.01% | 0.00% | 0.285 |
| unrelated_line | 50.02% ± 0.09% | 4,173,886.8 | 2,087,778.2 | 7.33% | 92.63% | 0.240 |
| no_victim | 49.99% ± 0.13% | 9,784,254.3 | 4,891,050.9 | 0.93% | 99.10% | 0.102 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 152 | 164 | 0.693408 | 2436129.623 |
| unrelated_line | 146 | 152 | 0.000000 | 1.768 |
| no_victim | 146 | 150 | 0.000001 | 9.420 |

Setup through frozen calibration: 0.017722 s (calibration collection alone: 0.005851 s).
Shared mapping: 16,384 bytes. Parent peak RSS: 2,304 KiB; victim peak RSS: 1,000 KiB.
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
