#!/usr/bin/env bash
set -euo pipefail
if [[ $# != 3 ]]; then
    echo "Usage: $0 ATTACKER_CPU VICTIM_CPU NEW_OUTPUT_DIRECTORY" >&2
    exit 2
fi
repo=$(cd -- "$(dirname -- "$0")/.." && pwd)
attacker=$1
victim=$2
out=$3
# A new directory prevents mixing old calibration with new results.
mkdir -- "$out"
out=$(cd -- "$out" && pwd)
printf '%s\n' "attacker_cpu=$attacker" "victim_cpu=$victim" > "$out/placement.txt"
hostname > "$out/hostname.txt"
uname -a > "$out/kernel.txt"
lscpu > "$out/lscpu.txt"
lscpu -e=CPU,CORE,SOCKET,ONLINE > "$out/topology.txt"
taskset -pc $$ > "$out/allowed_cpus.txt"
cc --version > "$out/compiler.txt"
cp -- "$repo/sc/ff_experiment.c" "$out/source.c"
cp -- "$repo/sc/ff_common.h" "$out/ff_common.h"
cc -std=c11 -O2 -Wall -Wextra -Werror "$out/source.c" -o "$out/ff_experiment"
sha256sum "$out/source.c" "$out/ff_common.h" "$out/ff_experiment" > "$out/sha256.txt"
objdump -d "$out/ff_experiment" > "$out/disassembly.txt"
set +e
timeout 90s "$out/ff_experiment" "$attacker" "$victim" "$out" > "$out/stdout.txt" 2> "$out/stderr.txt"
status=$?
set -e
printf '%s\n' "$status" > "$out/exit_status.txt"
if [[ $status != 0 ]]; then
    echo "Measurement failed (status $status). Keep this record; inspect $out/stderr.txt. No substitute technique used." >&2
    exit "$status"
fi
python3 "$repo/tools/report_ff.py" "$out" > "$out/report.md"
cat -- "$out/report.md"
