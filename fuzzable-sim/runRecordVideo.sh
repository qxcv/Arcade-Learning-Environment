#!/bin/env bash

set -xeuo pipefail

if [ "$#" -ne 2 ]; then
    echo "USAGE: $0 <rom name> <action file path>"
    exit 1
fi

rom_path="$1"
actions_path="$2"
tmpdir="$(mktemp -d)"
trap 'rm -rf -- "$tmpdir"' EXIT
rom_path="$(./romPath.sh "$rom_path")"
SDL_VIDEODRIVER=dummy ./build/recordVideo "$rom_path" "$tmpdir" < "$actions_path"
./joinVideo.sh "$tmpdir"
new_fn="agent-$(date -Iseconds).mp4"
mv -v agent.mp4 "$new_fn"
echo "Done, video in '$new_fn'"
