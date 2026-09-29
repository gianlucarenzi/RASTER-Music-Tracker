#!/usr/bin/env bash
# scripts/release.sh - create a numbered release tag (no rc suffix).
#
# Usage:  ./scripts/release.sh [version] [remote]
#   version  release version, e.g. "2.1"  (prompted if omitted)
#   remote   git remote (default: origin)
#
# After this, update RMT_BASE_VERSION in CMakeLists.txt to the next
# development version (e.g. "2.1") so that scripts/push.sh keeps the
# sequence going as v2.1-rc1, v2.1-rc2, ...

set -euo pipefail

REPO_ROOT="$(git -C "$(dirname "$0")" rev-parse --show-toplevel)"
cd "$REPO_ROOT"

VERSION=${1:-}
REMOTE=${2:-origin}

if [ -z "$VERSION" ]; then
    read -rp "Release version (e.g. 2.1): " VERSION
fi

if [ -z "$VERSION" ]; then
    echo "ERROR: version is required." >&2
    exit 1
fi

TAG="v${VERSION}"
BRANCH=$(git rev-parse --abbrev-ref HEAD)

echo "Branch  : $BRANCH"
echo "Release : $TAG"
echo ""

# Safety: refuse if there are uncommitted changes
if ! git diff --quiet HEAD; then
    echo "ERROR: uncommitted changes detected. Commit or stash them first." >&2
    exit 1
fi

read -rp "Create release tag '$TAG' and push? [Y/n] " REPLY
REPLY=${REPLY:-Y}
if [[ ! "$REPLY" =~ ^[Yy] ]]; then
    echo "Aborted."
    exit 0
fi

git tag "$TAG"
git push "$REMOTE" "$BRANCH"
git push "$REMOTE" "$TAG"
echo ""
echo "Done: released $TAG"
echo ""
echo "Next step: update RMT_BASE_VERSION in CMakeLists.txt for the next"
echo "  development cycle (e.g. set it to the next minor version)."
