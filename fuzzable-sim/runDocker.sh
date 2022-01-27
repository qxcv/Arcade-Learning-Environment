#!/bin/bash

set -euo pipefail

# TODO: make sure that cwd matches directory containing this file (otherwise
# the syntax for running binaries will make no sense)
above_dir="$( readlink -f -- "$( dirname -- "${BASH_SOURCE[0]}" )" )/../"
mount_dest="/home/wabbit/Arcade-Learning-Environment"
exec docker run -ti \
    -v "$above_dir":"$mount_dest" \
    -w "$mount_dest/fuzzable-sim" \
    humancompatibleai/wabbit:latest \
    bash -c 'eval "$@"' -- "$@"
