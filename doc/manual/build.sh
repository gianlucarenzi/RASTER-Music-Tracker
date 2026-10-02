#!/bin/bash
# Builds the manual of RITMO: doc/manual/ritmo-manual.pdf (XeLaTeX, pandoc and Python 3 are needed).
#
#   doc/manual/build.sh [--screenshots [path/to/ritmo]]
#
# --screenshots first takes the screenshots from the program (make-screenshots.sh); without it the
# images of doc/manual/images are used as they are.
set -eu
cd "$(dirname "$0")"
if [ "${1:-}" = "--screenshots" ]; then
    ./make-screenshots.sh ${2:+"$2"}
fi
mkdir -p generated

# the version of the manual: the tag when the commit is tagged, else RMT_BASE_VERSION of CMakeLists.txt
VERSION=$(git describe --tags --match 'v[0-9]*' --exact-match 2>/dev/null | sed 's/^v//' || true)
VERSION=${VERSION:-$(grep -m1 -oP 'set\(RMT_BASE_VERSION\s+"\K[^"]+' ../../CMakeLists.txt)}
DATE=$(date +"%B %Y")
cat >generated/version.tex <<TEX
\\newcommand{\\RitmoVersion}{${VERSION//_/\\_}}
\\newcommand{\\RitmoDate}{$DATE}
TEX

# the reference documents of doc/ as LaTeX
python3 md2tex.py ../rmt_en.md generated/reference.tex --shift 1 --from-heading "Menus, Toolbars and Keys" --to-heading "MIDI Input" --drop-rows "Menu Pokey / Channel"
python3 md2tex.py ../rmt_tracker_drivers.md generated/drivers.tex --shift 2
python3 md2tex.py ../rmt_scripting.md generated/scripting.tex --shift 1 --from-heading "RITMO Scripting"
python3 md2tex.py ../rmt_format.md generated/format.tex --shift 1 --from-heading 'RMT Module Format Version "1"' --to-heading 'RMT Module Format Version "2" (DRAFT 2026-01-06)'
cp ../midi.txt generated/midi.txt

# two runs, for the table of contents and the references; a third for the bookmarks
for i in 1 2 3; do
    xelatex -interaction=nonstopmode -halt-on-error ritmo-manual.tex >generated/xelatex.log 2>&1 || {
        tail -40 generated/xelatex.log
        echo "xelatex failed (see doc/manual/generated/xelatex.log)" >&2
        exit 1
    }
done
rm -f ritmo-manual.aux ritmo-manual.toc ritmo-manual.out ritmo-manual.log
echo "Wrote doc/manual/ritmo-manual.pdf ($(du -h ritmo-manual.pdf | cut -f1))"
