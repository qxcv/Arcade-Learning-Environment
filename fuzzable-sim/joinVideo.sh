#!/bin/bash
# A script to generate an ALE video with FFMpeg, *nix systems.

# -r ## specifies the frame rate
# -i record/%06d.png indicates we should use sequentially numbered frames in directory 'record'
# -i sound.wav indicates the location of the sound file
# -f mp4 specifies a MP4 format
# -c:a mp3 specifies the sound codec
# -c:v libx264 specifies the video codec
#

if [ "$#" -ne 1 ]; then
    echo "USAGE: $0 <dir>"
    exit 1
fi
ffmpeg -r 60 -i "$1"/%06d.png -i "$1"/sound.wav -f mp4 -c:a mp3 -c:v libx264 -loglevel error agent.mp4
