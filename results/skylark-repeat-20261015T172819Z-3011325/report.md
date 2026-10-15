| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |
|---|---:|---:|---:|---:|---:|---:|
| signal | 78.71% ± 3.52% | 1,591,487.9 | 1,252,707.9 | 42.14% | 0.44% | 0.628 |
| unrelated_line | 50.00% ± 0.00% | 1,797,589.8 | 898,794.9 | 0.00% | 100.00% | 0.556 |
| no_victim | 50.00% ± 0.00% | 2,479,929.9 | 1,239,964.9 | 0.00% | 100.00% | 0.403 |

| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |
|---|---:|---:|---:|---:|
| signal | 360 | 456 | 0.359360 | 571917.394 |
| unrelated_line | 456 | 480 | 0.000000 | 0.000 |
| no_victim | 456 | 456 | 0.000000 | 0.000 |

Setup through frozen calibration: 0.034871 s (calibration collection alone: 0.012425 s).
Shared mapping: 16,384 bytes. Parent peak RSS: 1,536 KiB; victim peak RSS: 672 KiB.
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
