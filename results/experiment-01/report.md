| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 67.00% ± 1.48% | 1,495,807.7 | 1,002,221.1 | 61.61% | 4.39% | 0.669 |
| unrelated_line | 49.90% ± 0.46% | 1,691,594.9 | 844,021.3 | 53.96% | 46.25% | 0.591 |
| no_victim | 50.03% ± 0.30% | 2,251,375.5 | 1,126,273.1 | 49.83% | 50.12% | 0.444 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 480 | 504 | 0.138572 | 207277.633 |
| unrelated_line | 480 | 504 | 0.000003 | 5.413 |
| no_victim | 504 | 504 | 0.000000 | 0.439 |

Setup time and memory use: unavailable (resources.csv was not recorded for this run).
Mean trial latency is batch elapsed time / trials, not an individually timed latency distribution.
Flush quantiles measure only the fenced probe in TSC ticks, not wall-clock microseconds.
Empirical information rate is a finite-sample classifier estimate, not channel capacity or proof of leakage;
it can be positively biased at chance and must be assessed against controls and repeated runs.

Correct bits/s = correct decisions / elapsed wall time; this includes chance guesses.
Chance accuracy is 50%. Compare signal with both controls and the variation across runs.
Above-chance calibration alone does not establish leakage. Fixed-order conditions may drift;
repeat the full experiment to assess reproducibility. Raw trials are temporally correlated;
do not treat every trial as an independent statistical replicate.
