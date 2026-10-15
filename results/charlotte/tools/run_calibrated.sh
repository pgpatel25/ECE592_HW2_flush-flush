#!/usr/bin/env bash
# Collect fresh machine-specific calibration, held-out trials, checks and figures.
set -euo pipefail
if [[ $# != 3 ]]; then
    echo "Usage: $0 ATTACKER_CPU VICTIM_CPU NEW_OUTPUT_DIRECTORY" >&2
    exit 2
fi
repo=$(cd -- "$(dirname -- "$0")/.." && pwd)
# Validate plotting dependencies before starting measurements.
python3 -c 'import matplotlib; import numpy'
bash "$repo/tools/run_local.sh" "$1" "$2" "$3"
out=$(cd -- "$3" && pwd)
cp -- "$repo/sc/ff_bench.c" "$out/bench_source.c"
cc -std=c11 -O2 -Wall -Wextra -Werror "$out/bench_source.c" -o "$out/ff_bench"
(cd -- "$out"; taskset -c "$1" ./ff_bench > calibration.txt)
sha256sum "$out/bench_source.c" "$out/ff_bench" >> "$out/sha256.txt"
python3 "$repo/tools/verify_evidence.py" "$out" > "$out/verification.txt"
python3 "$repo/tools/plot_ff.py" "$out"
echo "Calibration, trials, report, verification and figures saved in $out"
