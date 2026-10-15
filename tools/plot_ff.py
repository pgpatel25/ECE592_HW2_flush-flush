#!/usr/bin/env python3
"""Create assignment figures from one complete Flush+Flush result directory."""
import argparse
import csv
import pathlib

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np


MODES = ('signal', 'unrelated_line', 'no_victim')
LABELS = ('Signal', 'Different line\n(set unverified)', 'No victim')
COLORS = ('#0072B2', '#D55E00', '#009E73')
BIT_COLORS = ('#0072B2', '#CC79A7')


def read_rows(path):
    with path.open(newline='') as stream:
        reader = csv.DictReader(stream, skipinitialspace=True)
        if reader.fieldnames is None:
            return []
        reader.fieldnames = [field.strip() for field in reader.fieldnames]
        return [
            {key.strip(): value.strip() if value is not None else value
             for key, value in row.items() if key is not None}
            for row in reader
        ]


def save_figure(fig, output_dir, stem):
    fig.savefig(output_dir / f'{stem}.png', dpi=240, bbox_inches='tight')
    fig.savefig(output_dir / f'{stem}.pdf', bbox_inches='tight')
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('results_dir', type=pathlib.Path)
    args = parser.parse_args()
    root = args.results_dir
    summary = read_rows(root / 'summary.csv')
    if len(summary) != 30:
        parser.error('expected 30 summary rows (10 runs for each of 3 conditions)')
    if any(sum(row['mode'] == mode for row in summary) != 10 for mode in MODES):
        parser.error('summary must contain 10 runs for each expected condition')

    trials = {(mode, bit): [] for mode in MODES for bit in (0, 1)}
    for row in read_rows(root / 'trials.csv'):
        mode, bit = row['mode'], int(row['bit'])
        if mode not in MODES or bit not in (0, 1):
            parser.error('trials.csv contains an unknown condition or bit label')
        trials[(mode, bit)].append(int(row['tsc_ticks']))
    if any(len(values) != 50000 for values in trials.values()):
        parser.error('expected 50,000 raw trials per condition and known bit')

    threshold_values = {}
    for line in (root / 'threshold.txt').read_text().splitlines():
        if '=' in line:
            key, value = line.split('=', 1)
            threshold_values[key.strip()] = value.strip()
    threshold = int(threshold_values['threshold_tsc_ticks'])
    low_is_one = threshold_values['low_is_one'] == '1'
    output_dir = root / 'figures'
    output_dir.mkdir(parents=True, exist_ok=True)

    fig, ax = plt.subplots(figsize=(8.2, 4.8), constrained_layout=True)
    jitter = np.linspace(-0.17, 0.17, 10)
    for index, (mode, label, color) in enumerate(zip(MODES, LABELS, COLORS)):
        runs = sorted((row for row in summary if row['mode'] == mode),
                      key=lambda row: int(row['run']))
        accuracies = np.array([float(row['accuracy']) * 100 for row in runs])
        ax.scatter(index + jitter, accuracies, color=color, alpha=0.78, s=34,
                   edgecolors='white', linewidths=0.45, zorder=3)
        ax.errorbar(index, accuracies.mean(), yerr=accuracies.std(ddof=1),
                    fmt='D', color=color, markeredgecolor='white',
                    capsize=5, markersize=7, linewidth=1.5, zorder=4,
                    label=f'{label} mean +/- 1 SD')
    ax.axhline(50, color='#555555', linestyle='--', linewidth=1.2,
               label='Balanced-guess chance (50%)')
    ax.set_xticks(range(len(LABELS)))
    ax.set_xticklabels(LABELS)
    ax.set_ylim(40, 100)
    ax.set_ylabel('Held-out accuracy (%)')
    ax.set_title('Flush+Flush classification accuracy across runs')
    ax.grid(axis='y', color='#DDDDDD', linewidth=0.7)
    ax.set_axisbelow(True)
    ax.legend(frameon=False, fontsize=8, ncol=2, loc='upper left')
    save_figure(fig, output_dir, 'accuracy_by_condition')

    totals = {}
    for mode in MODES:
        rows = [row for row in summary if row['mode'] == mode]
        totals[mode] = {
            key: sum(int(row[key]) for row in rows)
            for key in ('tp', 'tn', 'fp', 'fn', 'correct')
        }
        totals[mode]['seconds'] = sum(float(row['seconds']) for row in rows)

    x = np.arange(len(MODES))
    width = 0.34
    fig, axes = plt.subplots(1, 2, figsize=(10.2, 4.6), constrained_layout=True)
    for offset, metric, label, color in (
            (-width / 2, 'fpr', 'False-positive rate', '#D55E00'),
            (width / 2, 'fnr', 'False-negative rate', '#0072B2')):
        rates = []
        for mode in MODES:
            counts = totals[mode]
            denominator = counts['fp'] + counts['tn'] if metric == 'fpr' else counts['fn'] + counts['tp']
            numerator = counts['fp'] if metric == 'fpr' else counts['fn']
            rates.append(100 * numerator / denominator)
        axes[0].bar(x + offset, rates, width, label=label, color=color)
    axes[0].set_xticks(x)
    axes[0].set_xticklabels(LABELS)
    axes[0].set_ylim(0, 100)
    axes[0].set_ylabel('Rate (%)')
    axes[0].set_title('Classifier errors')
    axes[0].legend(frameon=False, fontsize=8)
    axes[0].grid(axis='y', color='#DDDDDD', linewidth=0.7)
    axes[0].set_axisbelow(True)

    trial_rates = [100000 / totals[mode]['seconds'] / 1e6 for mode in MODES]
    correct_rates = [totals[mode]['correct'] / totals[mode]['seconds'] / 1e6 for mode in MODES]
    axes[1].bar(x - width / 2, trial_rates, width, label='Trials/s', color='#009E73')
    axes[1].bar(x + width / 2, correct_rates, width, label='Correct decisions/s', color='#E69F00')
    axes[1].set_xticks(x)
    axes[1].set_xticklabels(LABELS)
    axes[1].set_ylabel('Millions per second')
    axes[1].set_title('Observed rates')
    axes[1].legend(frameon=False, fontsize=8)
    axes[1].grid(axis='y', color='#DDDDDD', linewidth=0.7)
    axes[1].set_axisbelow(True)
    fig.suptitle('Error rates and observed throughput')
    fig.text(0.5, -0.015,
             'Correct decisions/s includes chance guesses. No-victim timing omits the victim handshake.',
             ha='center', fontsize=8)
    save_figure(fig, output_dir, 'errors_and_throughput')

    all_ticks = np.concatenate([np.asarray(values) for values in trials.values()])
    low, high = np.percentile(all_ticks, [0.5, 99.5])
    bins = np.linspace(low, high, 45)
    fig, axes = plt.subplots(1, 3, figsize=(12.2, 4.2), sharey=True, constrained_layout=True)
    for ax, mode, label in zip(axes, MODES, LABELS):
        for bit, color in enumerate(BIT_COLORS):
            ax.hist(trials[(mode, bit)], bins=bins, density=True, histtype='step',
                    linewidth=1.6, color=color, label=f'Known bit {bit}')
        ax.axvline(threshold, color='#222222', linestyle='--', linewidth=1.2,
                   label=f'Frozen threshold: {threshold}')
        ax.set_title(label)
        ax.set_xlabel('Fenced flush duration (TSC ticks)')
        ax.grid(axis='y', color='#DDDDDD', linewidth=0.7)
        ax.set_axisbelow(True)
    axes[0].set_ylabel('Density')
    axes[0].legend(frameon=False, fontsize=7)
    direction = 'low timings predict bit 1' if low_is_one else 'high timings predict bit 1'
    fig.suptitle(f'Probe timing by known bit ({direction})')
    save_figure(fig, output_dir, 'timing_distributions')

    resource_path = root / 'resources.csv'
    if resource_path.exists():
        resource_rows = read_rows(resource_path)
        if resource_rows:
            resource = resource_rows[0]
            fig, axes = plt.subplots(1, 2, figsize=(8.6, 4.2), constrained_layout=True)
            axes[0].bar(('Setup', 'Calibration'),
                        (float(resource['setup_seconds']),
                         float(resource['calibration_collection_seconds'])),
                        color=('#0072B2', '#56B4E9'))
            axes[0].set_ylabel('Seconds')
            axes[0].set_title('Setup cost')
            memory_kib = (int(resource['shared_mapping_bytes']) / 1024,
                          int(resource['parent_peak_rss_kib']),
                          int(resource['victim_peak_rss_kib']))
            axes[1].bar(('Shared mapping', 'Parent peak RSS', 'Victim peak RSS'),
                        memory_kib, color=('#009E73', '#E69F00', '#CC79A7'))
            axes[1].set_ylabel('KiB')
            axes[1].set_title('Memory measurements')
            axes[1].tick_params(axis='x', labelrotation=12)
            fig.suptitle('Setup and memory (single recorded invocation)')
            fig.text(0.5, -0.015,
                     'RSS includes runtime overhead; adding process RSS double-counts shared pages.',
                     ha='center', fontsize=8)
            save_figure(fig, output_dir, 'setup_and_memory')
    else:
        print('No resource figure: resources.csv is missing; setup and memory were not recorded.')

    print(f'Figures written to {output_dir}')
    for stem in ('accuracy_by_condition', 'errors_and_throughput', 'timing_distributions'):
        print(f'  {stem}.png and {stem}.pdf')
    if resource_path.exists():
        print('  setup_and_memory.png and setup_and_memory.pdf (when resource data is valid)')


if __name__ == '__main__':
    main()