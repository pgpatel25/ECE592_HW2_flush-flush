| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 100.00% ± 0.01% | 1,169,218.8 | 1,169,160.4 | 0.00% | 0.01% | 0.855 |
| unrelated_line | 50.00% ± 0.00% | 1,190,764.8 | 595,382.4 | 0.00% | 100.00% | 0.840 |
| no_victim | 50.00% ± 0.00% | 1,785,847.5 | 892,923.7 | 0.00% | 100.00% | 0.560 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 480 | 528 | 0.999263 | 1168357.674 |
| unrelated_line | 508 | 704 | 0.000000 | 0.000 |
| no_victim | 506 | 730 | 0.000000 | 0.000 |

Setup through frozen calibration: 0.025419 s (calibration collection alone: 0.017207 s).
Shared mapping: 16,384 bytes. Parent peak RSS: 912 KiB; victim peak RSS: 684 KiB.
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
