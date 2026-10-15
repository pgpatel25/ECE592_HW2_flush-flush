#!/usr/bin/env python3
"""Verify stored counts, classification, summary totals, and snapshot hashes."""
import csv
import hashlib
import sys
from collections import Counter, defaultdict
from pathlib import Path

p = Path(sys.argv[1]).resolve()
def rows(name):
    with (p / name).open() as f:
        return list(csv.DictReader(f))
def require(ok, message):
    if not ok:
        raise SystemExit('FAIL: ' + message)
require((p / 'exit_status.txt').read_text().strip() == '0', 'experiment exit status')
rule = dict(line.split('=', 1) for line in (p / 'threshold.txt').read_text().splitlines())
t = int(rule['threshold_tsc_ticks'])
low = int(rule['low_is_one'])
def predict(ticks):
    return low if int(ticks) <= t else 1-low
cal = rows('calibration.csv')
require(Counter(int(r['bit']) for r in cal) == {0:10000, 1:10000}, 'calibration class counts')
require([int(r['trial']) for r in cal] == list(range(20000)), 'calibration indices')
correct = sum(predict(r['tsc_ticks']) == int(r['bit']) for r in cal)
require(abs(correct/20000 - float(rule['calibration_accuracy'])) < 0.000001, 'calibration accuracy')
# Independently check that calibration chose a minimum-error threshold/direction.
bins = defaultdict(Counter)
for r in cal:
    bins[int(r['tsc_ticks'])][int(r['bit'])] += 1
zeros = ones = 0
best = 10000
for tick, counts in sorted(bins.items()):
    zeros += counts[0]
    ones += counts[1]
    best = min(best, zeros + 10000-ones, ones + 10000-zeros)
require(20000-correct == best, 'minimum calibration error')
groups = defaultdict(list)
for r in rows('trials.csv'):
    groups[r['mode'], int(r['run'])].append(r)
expected = {(mode, run) for mode in ('signal','unrelated_line','no_victim') for run in range(1,11)}
require(set(groups) == expected, 'three conditions, ten runs each')
summaries = rows('summary.csv')
require(len(summaries) == 30, 'summary row count')
sums = {(r['mode'],int(r['run'])):r for r in summaries}
require(set(sums) == expected, 'summary condition/run IDs')
for key, data in groups.items():
    require(Counter(int(r['bit']) for r in data) == {0:5000,1:5000}, f'{key} balanced trials')
    require([int(r['trial']) for r in data] == list(range(10000)), f'{key} trial indices')
    counts = Counter()
    for r in data:
        bit, pred = int(r['bit']), predict(r['tsc_ticks'])
        require(pred == int(r['prediction']), f'{key} frozen-rule prediction')
        counts[('tp' if pred else 'fn') if bit else ('fp' if pred else 'tn')] += 1
    summary = sums[key]
    for field in ('tp','tn','fp','fn'):
        require(counts[field] == int(summary[field]), f'{key} {field}')
    require(int(summary['correct']) == counts['tp']+counts['tn'], f'{key} correct count')
    require(int(summary['trials']) == 10000, f'{key} summary trial count')
    require(float(summary['seconds']) > 0, f'{key} elapsed time')
for line in (p / 'sha256.txt').read_text().splitlines():
    digest, filename = line.split(None, 1)
    artifact = p / Path(filename.strip()).name
    require(hashlib.sha256(artifact.read_bytes()).hexdigest() == digest, 'hash '+artifact.name)
source = (p / 'source.c').read_text()
require(source.index('fsync(fileno(frozen))') < source.index('OPEN_FILE(trials,'), 'freeze before test collection')
resource = rows('resources.csv')[0]
for key in ('setup_seconds','calibration_collection_seconds','shared_mapping_bytes','parent_peak_rss_kib','victim_peak_rss_kib'):
    require(float(resource[key]) > 0, 'positive resource measurement '+key)
print('PASS: 20,000 calibration observations (10,000/class).')
print('PASS: minimum-error calibration rule; freeze precedes test collection in snapshot source.')
print(f'Frozen rule: predict access when ticks {"<=" if low else ">"} {t}.')
print('PASS: 300,000 test observations; 10 runs/condition; 5,000 labels/class/run.')
print('PASS: all predictions and confusion counts match the frozen rule and raw data.')
print('PASS: source/header/executable SHA-256 and positive resource measurements.')
print('LIMIT: stored data cannot prove collection freshness independently; source and recorded seed document the protocol.')
print('LIMIT: cache-set independence and multi-host coverage require separate evidence.')
