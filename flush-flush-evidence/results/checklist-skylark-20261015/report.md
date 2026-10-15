| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 77.25% ± 2.82% | 1,626,979.2 | 1,256,922.8 | 45.49% | 0.00% | 0.615 |
| unrelated_line | 50.13% ± 0.21% | 1,911,135.5 | 958,014.0 | 5.34% | 94.40% | 0.523 |
| no_victim | 50.03% ± 0.18% | 2,685,373.0 | 1,343,572.7 | 6.45% | 93.48% | 0.372 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 432 | 456 | 0.348038 | 566251.291 |
| unrelated_line | 408 | 432 | 0.000023 | 43.670 |
| no_victim | 408 | 432 | 0.000001 | 3.478 |

Setup through frozen calibration: 0.032593 s (calibration collection alone: 0.012060 s).
Shared mapping: 16,384 bytes. Parent peak RSS: 1,536 KiB; victim peak RSS: 684 KiB.
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
