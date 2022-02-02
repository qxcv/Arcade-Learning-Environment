#!/bin/bash

# Find the path to a rom supplied by name

set -euo pipefail

supported_games="./games.txt"
rom_dir="${HOME}/etc/atari-roms/"
md5_path="${rom_dir}/md5sums.txt"
rom_name="$*"

# check that we were given a ROM name to look up
# TODO: maybe rewrite the check so that it first verifies that the ROM is in
# supported_games?
if [ -z "$rom_name" ]; then
    echo "USAGE: $0 rom_name" > /dev/stderr
    echo "Supported ROM names:" > /dev/stderr
    cut -d ' ' -f 1 < "$supported_games" > /dev/stderr
    exit 1
fi

if [ ! -f "$md5_path" ]; then
    # remake the index of ROM paths
    find "$rom_dir" -name '*.bin' \
        | while read f; do
            md5sum="$(md5sum -b "$f" | cut -d ' ' -f 1)"
            relpath="$(realpath --relative-to "$rom_dir" "$f")"
            echo "$md5sum *$relpath"
        done > "$md5_path"
fi

# remove special chars so as not to confuse grep
rom_name="$(echo "$rom_name" | sed -E 's/[^a-zA-Z0-9]//g')"
md5="$(grep "^${rom_name} " "$supported_games" | cut -d ' ' -f 2 || echo)"
if [[ ! "$md5" =~ ^[a-z0-9]{32}$ ]]; then
    if [ ! -z "$md5" ]; then
        echo "Possible internal error: md5='$md5'" > /dev/stderr
    fi
    echo "No match for ROM '$rom_name'" > /dev/stderr
    exit 1
fi

echo "ROM name ${rom_name} has MD5 ${md5}" > /dev/stderr

rom_path_base="$(grep "^${md5} *" "$md5_path" | cut -d '*' -f 2- | head -n 1 || echo)"
rom_path_full="$(readlink -f "$rom_dir/$rom_path_base")"
if [ -z "$rom_path_full" ]; then
    echo "Could not find a ROM with MD5 ${md5} in ${md5_path}"
    exit 1
fi

echo "$rom_path_full"
exit 0  # make it explicit
