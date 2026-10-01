#!/bin/bash
# Runs the reference scripts of test-resources/scripts through "rmt /SCRIPT:",
# twice each into separate folders: the exit code must be 0, the expected files
# must exist and both runs must produce byte-identical files (WAV included).
#
#   scripts/test-scripting.sh [path/to/rmt]      (default: build-qt/out/rmt)
set -u
cd "$(dirname "$0")/.."
RMT=${1:-build-qt/out/rmt}
[ -x "$RMT" ] || { echo "No such program: $RMT" >&2; exit 2; }
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
# the settings (tuning, driver...) are the defaults, not those of the user running the test
export XDG_CONFIG_HOME="$WORK/config"
fail=0

run() { # script output-folder
    RMT_SCRIPT_OUTPUT="$2" "$RMT" "/SCRIPT:test-resources/scripts/$1.rmtscript" >"$2.log" 2>&1
}

check() { # name expected-files...
    local name=$1; shift
    run "$name" "$WORK/$name-a" || { echo "FAIL $name: exit code $?"; tail -5 "$WORK/$name-a.log"; fail=1; return; }
    run "$name" "$WORK/$name-b" || { echo "FAIL $name (second run): exit code $?"; fail=1; return; }
    for f in "$@"; do
        [ -s "$WORK/$name-a/$f" ] || { echo "FAIL $name: $f is missing or empty"; fail=1; }
    done
    if diff -r "$WORK/$name-a" "$WORK/$name-b" >/dev/null; then
        echo "ok   $name: $(ls "$WORK/$name-a" | wc -l) files, both runs identical"
    else
        echo "FAIL $name: the two runs differ"; diff -rq "$WORK/$name-a" "$WORK/$name-b"; fail=1
    fi
}

check delta delta.rmt delta.txt delta.rmw d1.rmt d1.asm d1.sapr d1.lzss d1.sap d1.xex d1_player.asm d1.wav d2.sap d2.xex d3.sapr
check midi midi.rmt midi.txt midi.rmw

# the failure paths: a syntax error is exit code 2, a failing command 1
printf 'open "unterminated\n' >"$WORK/syntax.rmtscript"
"$RMT" "/SCRIPT:$WORK/syntax.rmtscript" >/dev/null 2>&1; code=$?
[ $code -eq 2 ] && echo "ok   syntax error: exit code 2" || { echo "FAIL syntax error: exit code $code, expected 2"; fail=1; }
printf 'open nothere.rmt\n' >"$WORK/missing.rmtscript"
"$RMT" "/SCRIPT:$WORK/missing.rmtscript" >/dev/null 2>&1; code=$?
[ $code -eq 1 ] && echo "ok   failing command: exit code 1" || { echo "FAIL failing command: exit code $code, expected 1"; fail=1; }

exit $fail
