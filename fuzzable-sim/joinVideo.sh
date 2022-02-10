#!/bin/bash
# A script to generate an ALE video with FFMpeg, *nix systems.

# -r ## specifies the frame rate
# -i record/%06d.png indicates we should use sequentially numbered frames in directory 'record'
# -i sound.wav indicates the location of the sound file
# -f mp4 specifies a MP4 format
# -c:a mp3 specifies the sound codec
# -c:v libx264 specifies the video codec
#

set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "USAGE: $0 <dir>"
    exit 1
fi
dir="$1"

# Note that -nostdin is required so that this script can be executed in 'while
# read' loops (see https://superuser.com/q/1492507); if you don't do that, then
# ffmpeg tries to consume part of stdin and consequently breaks the calling
# loop.
ffmpeg -nostdin -r 60 -i "$dir"/%06d.png -i "$dir"/sound.wav -f mp4 \
    -c:a mp3 -c:v libx264 -loglevel error "$dir/agent.mp4"
