#!/usr/bin/env bash
# Run from any working directory; all output is preserved under this package.
set -euo pipefail
repo=$(cd -- "$(dirname -- "$0")/.." && pwd)
if [[ $# != 2 && $# != 3 ]]; then
    echo "Usage: $0 ATTACKER_CPU VICTIM_CPU [NEW_OUTPUT_DIRECTORY]" >&2
    echo "Choose two different allowed CPU IDs using taskset -pc \$\$ and lscpu -e." >&2
    exit 2
fi
for command in cc python3 timeout objdump sha256sum lscpu taskset; do
    command -v "$command" >/dev/null || { echo "Missing dependency: $command" >&2; exit 2; }
done
host=$(hostname -s)
run_stamp=$(date -u +%Y%m%dT%H%M%SZ)
mkdir -p "$repo/results"
out=${3:-"$repo/results/$host-$run_stamp-$$"}
exec bash "$repo/tools/run_ff.sh" "$1" "$2" "$out"
