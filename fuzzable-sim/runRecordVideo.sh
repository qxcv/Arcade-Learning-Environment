#!/bin/bash

set -euo pipefail

if [ "$#" -ne 3 ]; then
    echo "USAGE: $0 <rom name> <action file path> <output dir>"
    exit 1
fi

rom_path="$1"
actions_path="$2"
out_dir="$3"
tmpdir="$(mktemp -d)"
record_video_out="${tmpdir}/out.txt"
trap 'rm -rf -- "$tmpdir"' EXIT
rom_path="$(./romPath.sh "$rom_path")"
SDL_VIDEODRIVER=dummy ./build/recordVideo "$rom_path" "$tmpdir" \
               < "$actions_path" \
    |& tee -a "$record_video_out"
agent_mp4="$tmpdir/agent.mp4"
if [ -e "$agent_mp4" ]; then
    # if previous invocation of joinVideo.sh fails then agent.mp4 will still be
    # lying around, causing ffmpeg to bail out due to existing video
    rm -f "$agent_mp4"
fi
./joinVideo.sh "$tmpdir"
# extract "^Steps: <n>$", "^Reward: <n>$" "^  Cart Name: <name>$"
declare -A data_regexes=(
    ["steps"]='^Steps: +(.*)$'
    ["reward"]='^Reward: +(.*)$'
    ["cart_name"]='^ *Cart Name: +(.*?)(19[789][0-9]|$)'
)
declare -A extracted_data
for re_key in "${!data_regexes[@]}"; do
    re_value="${data_regexes[$re_key]}"
    re_match="$(sed -En "s/${re_value}/\\1/p" "$record_video_out" | head -n 1)"
    extracted_data["$re_key"]="$re_match"
done
cart_name="$(echo "${extracted_data['cart_name']}" | sed -E 's/[^a-zA-Z0-9]+/-/g' | sed -E 's/^-+|-+$//g')"
now="$(date -Iseconds)"
if [ ! -e "$out_dir" ]; then
    mkdir -p "$out_dir"
fi
new_fn="${out_dir}/agent-${cart_name}-with-${extracted_data['reward']}-rew-${extracted_data['steps']}-steps-${now}.mp4"
mv -v "$agent_mp4" "$new_fn"
echo "Done, video in '$new_fn'"
