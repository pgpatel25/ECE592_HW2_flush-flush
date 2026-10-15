#!/usr/bin/env python3
"""Summarize complete runs; correct bits/s includes chance successes."""
import csv
import math
import pathlib
import statistics
import sys

root = pathlib.Path(sys.argv[1])

def read_rows(path):
    with path.open(newline='') as stream:
        reader = csv.DictReader(stream, skipinitialspace=True)
        if reader.fieldnames is None:
            return []
        reader.fieldnames = [name.strip() for name in reader.fieldnames]
        return [
            {key.strip(): value.strip() if value is not None else value
             for key, value in row.items() if key is not None}
            for row in reader
        ]

rows = read_rows(root / 'summary.csv')
if len(rows) != 30:
    sys.exit('Incomplete dataset: expected 30 summary rows')
print('| Condition | Accuracy, mean ± run SD | Trials/s | Correct bits/s | FPR | FNR | Mean trial µs |')
print('|---|---:|---:|---:|---:|---:|---:|')
raw = {mode: [] for mode in ('signal', 'unrelated_line', 'no_victim')}
counts = {mode: 0 for mode in raw}
with (root / 'trials.csv').open() as stream:
    for row in read_rows(root / 'trials.csv'):
        raw[row['mode']].append(int(row['tsc_ticks']))
        counts[row['mode']] += 1
if any(n != 100000 for n in counts.values()):
    sys.exit('Incomplete raw dataset: expected 100,000 trials per condition')
information = {}

for mode in ('signal', 'unrelated_line', 'no_victim'):
    runs = [r for r in rows if r['mode'] == mode]
    if {int(r['run']) for r in runs} != set(range(1, 11)) or len(runs) != 10:
        sys.exit(f'Incomplete runs for {mode}')
    if any(int(r['trials']) != 10000 or
           sum(int(r[k]) for k in ('tp', 'tn', 'fp', 'fn')) != 10000 or
           int(r['tp']) + int(r['fn']) != 5000 or
           int(r['tn']) + int(r['fp']) != 5000 for r in runs):
        sys.exit(f'Invalid trial counts for {mode}')
    acc = [int(r['correct']) / int(r['trials']) for r in runs]
    seconds = sum(float(r['seconds']) for r in runs)
    rate = 100000 / seconds
    correct_rate = sum(int(r['correct']) for r in runs) / seconds
    tp, tn, fp, fn = (sum(int(r[k]) for r in runs) for k in ('tp', 'tn', 'fp', 'fn'))
    fpr = fp / (fp + tn)
    fnr = fn / (fn + tp)
    print(f'| {mode} | {statistics.mean(acc):.2%} ± {statistics.stdev(acc):.2%} | '
          f'{rate:,.1f} | {correct_rate:,.1f} | {fpr:.2%} | {fnr:.2%} | {1e6/rate:.3f} |')
    # Empirical mutual information I(known bit; classified bit), including asymmetry.
    cells = ((tn, fp), (fn, tp))
    total = tp + tn + fp + fn
    mi = 0.0
    for x in range(2):
        for y in range(2):
            n = cells[x][y]
            nx, ny = sum(cells[x]), cells[0][y] + cells[1][y]
            if n:
                mi += n / total * math.log2(n * total / (nx * ny))
    information[mode] = (mi, mi * rate)
print('\n| Condition | Flush p50 TSC ticks | Flush p95 TSC ticks | Empirical information bits/trial | Information rate bits/s |')
print('|---|---:|---:|---:|---:|')
for mode, ticks in raw.items():
    ticks.sort()
    mi, rate = information[mode]
    print(f'| {mode} | {ticks[math.ceil(.50 * len(ticks))-1]} | '
          f'{ticks[math.ceil(.95 * len(ticks))-1]} | {mi:.6f} | {rate:.3f} |')
resource_path = root / 'resources.csv'
resource_rows = read_rows(resource_path) if resource_path.exists() else []
if resource_rows:
    resource = resource_rows[0]
    print(f"\nSetup through frozen calibration: {float(resource['setup_seconds']):.6f} s "
        f"(calibration collection alone: {float(resource['calibration_collection_seconds']):.6f} s).")
    print(f"Shared mapping: {int(resource['shared_mapping_bytes']):,} bytes. "
        f"Parent peak RSS: {int(resource['parent_peak_rss_kib']):,} KiB; "
        f"victim peak RSS: {int(resource['victim_peak_rss_kib']):,} KiB.")
    print('Peak RSS values are per process, include runtime overhead, and double-count shared pages if added.')
else:
    print('\nSetup time and memory use: unavailable (resources.csv was not recorded for this run).')
print('Mean trial latency is batch elapsed time / trials, not an individually timed latency distribution.')
print('Flush quantiles measure only the fenced probe in TSC ticks, not wall-clock microseconds.')
print('Empirical information rate is a finite-sample classifier estimate, not channel capacity or proof of leakage;')
print('it can be positively biased at chance and must be assessed against controls and repeated runs.')
print('\nCorrect bits/s = correct decisions / elapsed wall time; this includes chance guesses.')
print('Chance accuracy is 50%. Compare signal with both controls and the variation across runs.')
print('Above-chance calibration alone does not establish leakage. Fixed-order conditions may drift;')
print('repeat the full experiment to assess reproducibility. Raw trials are temporally correlated;')
print('do not treat every trial as an independent statistical replicate.')
