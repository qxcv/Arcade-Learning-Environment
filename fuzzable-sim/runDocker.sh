#!/bin/bash

set -euo pipefail

# TODO: add check that cwd matches directory containing this file (otherwise
# the syntax for running binaries will make no sense)
above_dir="$( readlink -f -- "$( dirname -- "${BASH_SOURCE[0]}" )" )/../"
host_rom_dir="${HOME}/etc/atari-roms"
mount_repo_dest="/home/wabbit/Arcade-Learning-Environment"
mount_rom_dest="/home/wabbit/etc/atari-roms"
host_gid="$(id -g)"
host_uid="$(id -u)"

# update base image to have UID/GID that matches user on host
latest_wabbit_base="humancompatibleai/wabbit:latest"
base_hash="$(docker image ls -q "$latest_wabbit_base" | head -n 1)"
if [ -z "$base_hash" ]; then
  echo "Could not find base image '$latest_wabbit_base'. Do you need to do docker build/pull?"
fi
latest_wabbit_uid_gid="$latest_wabbit_base----local-cache-uid-$host_uid-gid-$host_gid-hash-$base_hash"
mod_cmd="groupmod -g '$host_gid' wabbit && chown -R :wabbit ~wabbit && usermod -u '$host_uid' wabbit"
if [ -z "$(docker image ls -q "$latest_wabbit_uid_gid")" ]; then
  echo "Could not find cached image $latest_wabbit_uid_gid, creating it"
  # note that `tr` dies when combined with `head -c`, which combined with `set
  # -o pipefail` causes the whole script to die.
  rand_str="$( ( tr -c -d '[:alnum:]' < /dev/urandom || true ) | head -c 16)"
  tmp_container_usermod="tempc1-$rand_str"
  set -x
  # change `wabbit` account's UID/GID to match host UID/GID (run as root)
  docker run -u root --name "$tmp_container_usermod" "$latest_wabbit_base" bash -c "$mod_cmd && mkdir ~wabbit/Arcade-Learning-Environment && chown wabbit:wabbit ~wabbit/Arcade-Learning-Environment"
  docker commit --change "USER wabbit" "$tmp_container_usermod" "$latest_wabbit_uid_gid"
  docker rm "$tmp_container_usermod"
  set +x
fi

if tty -s; then
    interact_flags=("-ti")
else
    declare -a interact_flags
fi

# FIXME(sam): use `root` user when running with user-mode docker (which maps
# `root` UID to host user UID) and `wabbit` otherwise.
exec docker run "${interact_flags[@]}" --rm \
    -v "$above_dir:$mount_repo_dest:rw" \
    -v "$host_rom_dir:$mount_rom_dest:ro" \
    -w "$mount_repo_dest/fuzzable-sim" \
    -u root \
    "$latest_wabbit_uid_gid" \
    bash -c 'ln -s ~wabbit/etc /root/etc && eval -- "$@"' -- "$@"
