| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 61.52% ± 11.10% | 3,446,261.2 | 2,120,312.2 | 76.77% | 0.18% | 0.290 |
| unrelated_line | 49.93% ± 0.34% | 4,425,048.6 | 2,209,249.7 | 83.74% | 16.41% | 0.226 |
| no_victim | 50.06% ± 0.22% | 10,256,078.9 | 5,134,090.5 | 93.66% | 6.22% | 0.098 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 125 | 131 | 0.120268 | 414476.383 |
| unrelated_line | 122 | 125 | 0.000003 | 12.789 |
| no_victim | 123 | 125 | 0.000004 | 43.763 |

Setup through frozen calibration: 0.018035 s (calibration collection alone: 0.009587 s).
Shared mapping: 16,384 bytes. Parent peak RSS: 2,304 KiB; victim peak RSS: 904 KiB.
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
