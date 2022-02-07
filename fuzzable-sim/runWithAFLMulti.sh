#!/bin/bash

set -euo pipefail

# Args:
# $0 <session-name> <num cores> <args to runWithAFL.sh>

if [ "$#" -ne 3 ]; then
    echo "USAGE: $0 session-name num-procs game-name"
    exit 1
fi

session_name="$1"
shift
num_procs="$1"
shift

# TODO(sam) validate that session_name is just [a-zA-Z0-9_-] and that num_procs
# is a natural.

afl_cmd() {
    # passing "$@" around as a string is doomed to failure eventually, but
    # whatever
    echo "./runDocker.sh ./runWithAFL.sh -o fuzzer_findings_$session_name -i $*"
}

tmux new-session -d -s "$session_name" -n master
tmux send-keys -t "=$session_name:=master" "$(afl_cmd 0 "$@")" Enter
for snum in $(seq 1 "$(( num_procs - 1 ))"); do
    tmux new-window -d -t "=$session_name" -n "slave$snum"
    tmux send-keys -t "=$session_name:=slave$snum" \
        "$(afl_cmd "$snum" "$@")" Enter
done
cat <<ALL_DONE
Launch of $num_procs processes complete, use 'tmux a -t $session_name' to track
progress, or use 'ctrl+b w' to select windows/sessions if you are attached to a
tmux session already.
ALL_DONE
