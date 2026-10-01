#!/bin/bash
# Regenerates the documents that come from the program itself, with the script commands
# "dump actions" and "dump notekeys":
#   doc/rmt_action_infos.md   every menu item and toolbar button with its key and description
#   doc/rmt_note_keys.md      the note keys of the QWERTY, QWERTZ and AZERTY layouts
#
#   scripts/make-docs.sh [path/to/rmt]      (default: build-qt/out/rmt)
set -eu
cd "$(dirname "$0")/.."
RMT=${1:-build-qt/out/rmt}
[ -x "$RMT" ] || { echo "No such program: $RMT" >&2; exit 2; }
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
export XDG_CONFIG_HOME="$WORK/config"      # the defaults, not the settings of the user
cat >"$WORK/docs.rmtscript" <<SCRIPT
set overwrite yes
dump actions $PWD/doc/rmt_action_infos.md
dump notekeys $PWD/doc/rmt_note_keys.md
SCRIPT
"$RMT" "/SCRIPT:$WORK/docs.rmtscript"
