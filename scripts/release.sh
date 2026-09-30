#!/usr/bin/env bash
# scripts/release.sh - create a numbered release tag (no rc suffix).
#
# Usage:  ./scripts/release.sh [version] [remote]
#   version  release version, e.g. "2.1"  (prompted if omitted)
#   remote   git remote (default: origin)
#
# The release notes must be written and committed first, in
# doc/release-notes/v<version>.md: the first line is "# <release title>", the
# rest is the body of the GitHub release. The CI builds only upload the
# packages, they do not write title or notes.
#
# After this, update RMT_BASE_VERSION in CMakeLists.txt to the next
# development version (e.g. "2.3") so that scripts/push.sh keeps the
# sequence going as v2.3-rc1, v2.3-rc2, ...

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
NOTES="doc/release-notes/${TAG}.md"

# Safety: the release notes must exist and be committed
if [ ! -f "$NOTES" ] || ! git ls-files --error-unmatch "$NOTES" >/dev/null 2>&1; then
    echo "ERROR: $NOTES is missing or not committed." >&2
    echo "  First line: '# <release title>', then the notes (see doc/release-notes/v2.2.md)." >&2
    exit 1
fi
TITLE=$(sed -n '1s/^# *//p' "$NOTES")
if [ -z "$TITLE" ]; then
    echo "ERROR: the first line of $NOTES must be '# <release title>'." >&2
    exit 1
fi
BODY=$(mktemp)
trap 'rm -f "$BODY"' EXIT
tail -n +2 "$NOTES" | sed '1{/^$/d}' > "$BODY"

echo "Branch  : $BRANCH"
echo "Release : $TAG"
echo "Title   : $TITLE"
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

# Title and notes of the GitHub release: the CI builds create it too (only
# to attach the packages), so edit it when it already exists.
if command -v gh >/dev/null 2>&1; then
    if gh release view "$TAG" >/dev/null 2>&1; then
        gh release edit "$TAG" --title "$TITLE" --notes-file "$BODY"
    elif ! gh release create "$TAG" --verify-tag --title "$TITLE" --notes-file "$BODY"; then
        gh release edit "$TAG" --title "$TITLE" --notes-file "$BODY"
    fi
else
    echo "WARNING: gh not found, set title and notes by hand:" >&2
    echo "  gh release edit $TAG --title \"$TITLE\" --notes-file $NOTES" >&2
fi
echo ""
echo "Done: released $TAG"
echo ""
echo "Next step: update RMT_BASE_VERSION in CMakeLists.txt for the next"
echo "  development cycle (e.g. set it to the next minor version)."
