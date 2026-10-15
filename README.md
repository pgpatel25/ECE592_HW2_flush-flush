# Flush+Flush: ECE 592 HW2, Section 2

This is the **Flush+Flush** portion of the group's three-attack PoC assignment, not the complete 20-point Section 2. The repository contains the original controlled two-process synthetic victim, implementation, calibration, recorded measurements, plots and reproduction tools. **No attack implementation, raw trials, thresholds or plots have been changed in this cleanup.**

## Repository map

| Path | Purpose |
|---|---|
| [sc/](sc/) | Original current C experiment (`ff_experiment.c`), standalone diagnostic calibration (`ff_bench.c`) and common header |
| [original/](original/) | Historical source retained for provenance |
| [tools/](tools/) | Collection, reporting, plotting and validation scripts |
| [results/COMPARISON.md](results/COMPARISON.md) | One concise machine-by-machine measurement index |
| [results/measurements.csv](results/measurements.csv) | Combined existing machine-level metrics |
| [results/<host>/](results/) | Unmodified machine-specific raw trials, calibration, environment, verification, figures |
| [docs/EXPERIMENT.md](docs/EXPERIMENT.md) | Protocol, commands and output dictionary |
| [docs/HANDOFF.md](docs/HANDOFF.md) | Integration checklist and provenance decisions |
| [docs/AI_DISCLOSURE.md](docs/AI_DISCLOSURE.md) | AI assistance and course-policy discrepancy |

## Running the existing experiment

Choose **two distinct CPU IDs allowed by your lab allocation**, verify using `taskset -pc $$` and `lscpu -e=CPU,CORE,SOCKET,ONLINE`. Never assume an idle CPU is allocated.

```bash
cc -std=c11 -O2 -Wall -Wextra sc/ff_bench.c -o sc/ff_bench
cc -std=c11 -O2 -Wall -Wextra sc/ff_experiment.c -o sc/ff_experiment
bash tools/run_calibrated.sh ATTACKER_CPU VICTIM_CPU results/NEW_RUN_NAME
```

The runner uses Linux x86-64, a C compiler, bash, Python 3, NumPy and Matplotlib (for figures). The destination must **not exist** already. To inspect previously collected evidence without new measurements:

```bash
python3 tools/verify_evidence.py results/skylark
python3 tools/report_ff.py results/skylark
```

## Measurements

Calibration has 10,000 known observations per class. Each of `signal`, `unrelated_line` and `no_victim` has ten fresh balanced runs of 10,000 trials. The victim-matched threshold and its polarity are frozen before held-out testing. The separate `ff_bench` result is diagnostic and is **not** reused as the experiment threshold.

TSC timing ticks are not necessarily CPU core cycles. Reported correct bits/s includes lucky guesses and does not prove information leakage. The no-victim control changes synchronization cost; the unrelated-line control was **not verified as unrelated-set**. A synchronized, known-bit PoC is not asynchronous AES key recovery. Preserve negative results and variation rather than retuning on held-out data.

The assignment PDF names Thunderbird, whereas these records include **Charlotte** as a claimed replacement; confirm course-staff authorization before calling this full assigned coverage. See the handoff and disclosure documents.
