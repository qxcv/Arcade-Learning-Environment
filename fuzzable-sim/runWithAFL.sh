#!/bin/bash

set -euo pipefail

TIMEOUT_MS="${TIMEOUT_MS:-1000}"

eval set -- "$(getopt -o i:o: -- "$@")"
run_id=0
findings_dir="fuzzer_findings"
while true; do
    case "$1" in
        -i)
            shift
            run_id="$1"
            shift
            ;;
        -o)
            shift
            findings_dir="$1"
            shift
            ;;
        --)
            shift
            break
            ;;
        *)
            break
            ;;
    esac
done
rom_path="$(./romPath.sh "$1")"
# TODO(sam): validate args
if [ "$run_id" -eq 0 ]; then
    # master
    echo "Launching master fuzzer (id $run_id)"
    par_args=("-M" "fuzzer$run_id")
else
    # slave
    echo "Launching slave fuzzer (id $run_id)"
    par_args=("-S" "fuzzer$run_id")
fi
AFL_SKIP_CPUFREQ=1 exec ../AFL/afl-fuzz -i test_cases -o "$findings_dir" \
    -t "$TIMEOUT_MS" "${par_args[@]}" -- ./build/fuzzableSim "$rom_path"
