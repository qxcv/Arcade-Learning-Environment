#!/bin/bash

set -euo pipefail

# Given a directory containing afl-fuzz output directories, this script
# enumerates the (unique) files in the queue of each fuzzer and turns each into
# an .mp4 file.

if [ "$#" -ne 3 ]; then
    echo "USAGE: $0 <game name> <input dir> <output dir>"
    exit 1
fi
game_name="$1"
in_dir="$2"
out_dir="$3"

echo "Will read data for game '$game_name' from '$in_dir', save to '$out_dir'"
if [ ! -e "$out_dir" ]; then
    mkdir -p "$out_dir" || true
fi

# Note that `IFS= read -d '' -r action_file` was the least-buggy way I could
# find of reading in a NUL-separated list of paths. What the components do:
#  - IFS= prevents `read` from stripping whitespace from beginning and end of
#    paths.
#  - `-d ''` treats NUL as the entry separator. Note that '' is indeed the empty
#    string; bash treats that as NUL, because all bash strings are
#    NUL-terminated.
#  - `-r` prevents `read` from interpreting backslashes in the input as escape
#    characters.

# find "$in_dir" -mindepth 3 -maxdepth 3 -path '*/queue/*' -name 'id:*' \
#     \( -not -name '*,sync:fuzzer*' \) \( -not -name '*,orig:*' \) -print0 | \
# while IFS= read -d '' -r action_file; do
find "$in_dir" -mindepth 3 -maxdepth 3 -path '*/queue/*' -name 'id:*' \
    \( -not -name '*,sync:fuzzer*' \) \( -not -name '*,orig:*' \) | \
while read action_file; do
    echo "Working on '$action_file'"
    ./runRecordVideo.sh "$game_name" "$action_file" "$out_dir"
    sleep 1
done
