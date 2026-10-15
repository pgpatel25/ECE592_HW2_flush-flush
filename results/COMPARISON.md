# Flush+Flush: comparison of previously recorded runs

These numbers are copied from existing `results/<host>/report.md` files; **no measurements were recollected or recomputed** for this index. Signal accuracy is mean ± across-run standard deviation, based on ten held-out runs of 10,000 bits. The control accuracies are from separately collected balanced trials.

| Host (original detailed report) | Signal accuracy | Signal trials/s | Unrelated-line accuracy | No-victim accuracy | Signal FPR | Signal FNR |
|---|---:|---:|---:|---:|---:|---:|
| [sunbird](../results/sunbird/report.md) | 98.13% ± 5.90% | 2,014,320.9 | 50.00% ± 0.00% | 50.00% ± 0.00% | 0.00% | 3.74% |
| [skylark](../results/skylark/report.md) | 77.25% ± 2.82% | 1,626,979.2 | 50.13% ± 0.21% | 50.03% ± 0.18% | 45.49% | 0.00% |
| [artemisia](../results/artemisia/report.md) | 100.00% ± 0.01% | 1,169,218.8 | 50.00% ± 0.00% | 50.00% ± 0.00% | 0.00% | 0.01% |
| [charnwood](../results/charnwood/report.md) | 99.55% ± 0.63% | 3,703,126.6 | 49.98% ± 0.05% | 50.00% ± 0.01% | 0.53% | 0.38% |
| [crux](../results/crux/report.md) | 61.52% ± 11.10% | 3,446,261.2 | 49.93% ± 0.34% | 50.06% ± 0.22% | 76.77% | 0.18% |
| [ookay](../results/ookay/report.md) | 92.99% ± 3.96% | 3,513,269.3 | 50.02% ± 0.09% | 49.99% ± 0.13% | 14.01% | 0.00% |
| [upgrade](../results/upgrade/report.md) | 82.30% ± 5.78% | 3,318,591.8 | 50.18% ± 0.46% | 50.05% ± 0.57% | 35.27% | 0.13% |
| [charlotte](../results/charlotte/report.md) | 50.01% ± 0.01% | 3,486,914.0 | 50.00% ± 0.01% | 50.00% ± 0.01% | 0.03% | 99.96% |

Machine-readable summary: [measurements.csv](measurements.csv).

## Underlying measurements and what to preserve

Each host directory keeps `calibration.csv` (20,000 samples), `threshold.txt` (frozen rule), `trials.csv` (300,000 raw, labeled trials), `summary.csv` (per-run confusion counts and rate), `report.md` (metrics and timing percentiles), `resources.csv` (setup and memory), and `figures/` where available. Source/header, executable, disassembly and SHA-256 records have been **retained in every host run**, even when repeated, because existing `verify_evidence.py` needs the snapshots. Supporting CPU placement/topology/compiler/metadata TXT records remain separate rather than being rewritten and losing their audit trail.

A genuinely separate Skylark run previously in the duplicated package is now at [skylark-repeat-20261015T172819Z-3011325](skylark-repeat-20261015T172819Z-3011325/); its raw observations are **not mixed** with the main Skylark dataset.

An accuracy near 50% is a negative or inconclusive observation, **not evidence of a successful leak**. Accuracy differences alone do not isolate cache properties. Results involve known bits, explicit synchronization, a non-verified unrelated-set control and fixed-order conditions. Correct bits/s includes chance successes. No-victim throughput is not directly comparable because it excludes the same handshake. The assignment lists Thunderbird; only Charlotte data exist here and the substitution requires course approval.
