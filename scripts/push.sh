#!/usr/bin/env bash
# scripts/push.sh - push the current branch and auto-create the next RC tag.
#
# Usage:  ./scripts/push.sh [remote]
#   remote  git remote to push to (default: origin)
#
# Tag scheme: v<BASE>-rc<N>  where BASE = RMT_BASE_VERSION from CMakeLists.txt
# and N is one more than the highest existing rcN tag for that base version.
# Example sequence: v2.0-rc1, v2.0-rc2, …
# When ready to release use scripts/release.sh instead.

set -euo pipefail

REMOTE=${1:-origin}
REPO_ROOT="$(git -C "$(dirname "$0")" rev-parse --show-toplevel)"
cd "$REPO_ROOT"

# Read the base version (e.g. "2.0") from CMakeLists.txt
BASE=$(grep -m1 -oP 'set\(RMT_BASE_VERSION\s+"\K[^"]+' CMakeLists.txt || true)
if [ -z "$BASE" ]; then
    echo "ERROR: could not read RMT_BASE_VERSION from CMakeLists.txt" >&2
    exit 1
fi

# Find the highest existing rcN for this base (none yet: grep fails, N = 1)
LAST_N=$(git tag -l "v${BASE}-rc*" | { grep -oP "(?<=-rc)[0-9]+$" || true; } | sort -n | tail -1)
NEXT_N=$(( ${LAST_N:-0} + 1 ))
NEW_TAG="v${BASE}-rc${NEXT_N}"

BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo "Branch : $BRANCH"
echo "Tag    : $NEW_TAG"
echo ""

# Confirm
read -rp "Push '$BRANCH' and create tag '$NEW_TAG'? [Y/n] " REPLY
REPLY=${REPLY:-Y}
if [[ ! "$REPLY" =~ ^[Yy] ]]; then
    echo "Aborted."
    exit 0
fi

git tag "$NEW_TAG"
git push "$REMOTE" "$BRANCH"
git push "$REMOTE" "$NEW_TAG"
echo ""
echo "Done: pushed $BRANCH and $NEW_TAG to $REMOTE"
