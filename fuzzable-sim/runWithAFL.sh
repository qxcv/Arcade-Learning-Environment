#!/bin/env bash

set -euo pipefail

rom_path="$(./romPath.sh "$@")"
AFL_SKIP_CPUFREQ=1 exec ../AFL/afl-fuzz -i test_cases -o fuzzer_findings \
    -- ./build/fuzzableSim "$rom_path"
