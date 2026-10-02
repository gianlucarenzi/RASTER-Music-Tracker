#!/usr/bin/env python3
"""RITMO icon: the "R" of the logo (IBM VGA 9x16 glyph, see make_logo.py) with
the raster bars, on a 32x32 grid of units with stepped corners.

Writes ritmo-icon.svg here and, in src/res (the build uses them),
ritmo-icon.png (256x256), ritmo-icon-512.png, ritmo-icon-32.png and ritmo.ico
(16, 32, 48, 64, 128 and 256 pixels).

    python3 make_icon.py
"""
import os

import make_logo as logo
from PIL import Image

N = 32
BIG = 2
BG, EDGE = logo.BG, "#3a3a8a"
grid = {}


def put(x, y, c):
    if 0 <= x < N and 0 <= y < N:
        grid[(x, y)] = c


for y in range(N):
    for x in range(N):
        put(x, y, BG)
# the stripes behind the letter
for i, c in enumerate(logo.STRIPES):
    for y in range(11 + i * 2, 11 + i * 2 + 2):
        for x in range(N):
            put(x, y, c)

# the letter, centred
ch = "R"
x0 = (N - 8 * BIG) // 2
y0 = (N - logo.LINES * BIG) // 2
letter = set()
for gy, row in enumerate(logo.FONT[ch]):
    for gx, v in enumerate(row):
        if v == "1":
            for dy in range(BIG):
                for dx in range(BIG):
                    letter.add((x0 + gx * BIG + dx, y0 + gy * BIG + dy))
for (x, y) in letter:
    put(x + 1, y + 1, logo.SHADOW)
for (x, y) in letter:
    for dx in (-1, 0, 1):
        for dy in (-1, 0, 1):
            if (x + dx, y + dy) not in letter:
                put(x + dx, y + dy, logo.OUTLINE)
hh = logo.LINES * BIG
for (x, y) in letter:
    put(x, y, logo.BARS[(y - y0) * len(logo.BARS) // hh])

# the lines at the top and at the bottom, like the logo
for x in range(N):
    put(x, 2, logo.ACCENT)
    put(x, 3, "#ffd23c")
    put(x, N - 4, "#ffd23c")
    put(x, N - 3, logo.ACCENT)

# the border and the stepped corners (transparent)
for i in range(N):
    for p in ((i, 0), (i, N - 1), (0, i), (N - 1, i)):
        put(p[0], p[1], EDGE)
for cx, cy in ((0, 0), (N - 1, 0), (0, N - 1), (N - 1, N - 1)):
    sx = 1 if cx == 0 else -1
    sy = 1 if cy == 0 else -1
    for dx, dy in ((0, 0), (1, 0), (2, 0), (0, 1), (0, 2), (1, 1)):
        grid.pop((cx + sx * dx, cy + sy * dy), None)


def rgba(c):
    return (int(c[1:3], 16), int(c[3:5], 16), int(c[5:7], 16), 255)


base = Image.new("RGBA", (N, N), (0, 0, 0, 0))
for (x, y), c in grid.items():
    base.putpixel((x, y), rgba(c))


def size(n):
    return base.resize((n, n), Image.NEAREST)


RES = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "src", "res")
size(256).save(os.path.join(RES, "ritmo-icon.png"))
size(512).save(os.path.join(RES, "ritmo-icon-512.png"))
size(32).save(os.path.join(RES, "ritmo-icon-32.png"))
# the sizes that are not a multiple of 32 are enlarged from the 32x32 grid by
# the nearest pixel: 16 and 48 are not perfect, the others are exact
size(256).save(os.path.join(RES, "ritmo.ico"), sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])

out = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {N} {N}" width="256" height="256" shape-rendering="crispEdges">',
       "<title>RITMO</title>"]
for y in range(N):
    x = 0
    while x < N:
        c = grid.get((x, y))
        n = 1
        while c and x + n < N and grid.get((x + n, y)) == c:
            n += 1
        if c:
            out.append(f'<rect x="{x}" y="{y}" width="{n}" height="1" fill="{c}"/>')
        x += n
out.append("</svg>")
open("ritmo-icon.svg", "w").write("\n".join(out) + "\n")
print("icon written")
