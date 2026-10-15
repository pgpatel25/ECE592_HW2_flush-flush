| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 50.01% ± 0.01% | 3,486,914.0 | 1,743,666.2 | 0.03% | 99.96% | 0.287 |
| unrelated_line | 50.00% ± 0.01% | 3,993,179.8 | 1,996,470.1 | 0.02% | 99.99% | 0.250 |
| no_victim | 50.00% ± 0.01% | 10,236,381.6 | 5,118,395.5 | 0.01% | 99.99% | 0.098 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 144 | 154 | 0.000008 | 26.781 |
| unrelated_line | 138 | 152 | 0.000004 | 15.332 |
| no_victim | 140 | 144 | 0.000004 | 37.317 |

Setup through frozen calibration: 0.066000 s (calibration collection alone: 0.015710 s).
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
