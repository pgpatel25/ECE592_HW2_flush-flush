| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 99.55% ± 0.63% | 3,703,126.6 | 3,686,388.4 | 0.53% | 0.38% | 0.270 |
| unrelated_line | 49.98% ± 0.05% | 4,372,206.5 | 2,185,316.2 | 0.22% | 99.82% | 0.229 |
| no_victim | 50.00% ± 0.01% | 11,138,702.9 | 5,569,351.5 | 0.01% | 99.99% | 0.090 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 136 | 144 | 0.958372 | 3548972.868 |
| unrelated_line | 126 | 128 | 0.000012 | 51.783 |
| no_victim | 126 | 128 | 0.000000 | 0.000 |

Setup through frozen calibration: 0.015119 s (calibration collection alone: 0.005542 s).
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
