#!/usr/bin/env python3
"""RITMO logo: pixel art, drawn on a grid of "units" (1 unit = 1 square pixel).

Writes ritmo-logo.svg (one <rect> per run of pixels, crisp edges) and
ritmo-logo.png (the same grid, scaled by SCALE with no smoothing).

    python3 make_logo.py
"""
from PIL import Image

import os

from PIL import Image, ImageDraw, ImageFont

W, H = 288, 156         # the canvas in units (32 cells of 9 pixels wide)
SCALE = 5               # PNG pixels per unit
BIG = 4                 # units per pixel of the big letters

# The glyphs are those of the IBM VGA 9x16 text mode font, "PxPlus" version of
# "The Ultimate Oldschool PC Font Pack" by VileR (CC BY-SA 4.0,
# https://int10h.org/oldschool-pc-fonts/): each character is a cell of 9x16
# pixels, here the 10 lines of the capitals (lines 2..11).
FONT_FILE = os.environ.get("VGA_FONT", os.path.expanduser("~/.fonts/PxPlus_IBM_VGA_9x16.ttf"))
CELL_W, CELL_H = 9, 16
TOP, LINES = 2, 10

BG = "#0a0a28"          # dark blue, like the Atari screen border
OUTLINE = "#000000"
SHADOW = "#2a1450"

# the raster bars of the letters, top to bottom (an Atari "kernel" effect)
BARS = ["#fff35a", "#ffd23c", "#ffa22e", "#ff6a2c", "#f03c78", "#c63cc4", "#7c50e8", "#4c84f4"]
# the stripes behind the letters
STRIPES = ["#16164a", "#1c1c60", "#242478", "#1c1c60", "#16164a"]
SUB = "#8fd0ff"         # subtitle
ACCENT = "#ff6a2c"

_font = ImageFont.truetype(FONT_FILE, 16)


class Glyphs(dict):
    def __missing__(self, ch):
        im = Image.new("1", (CELL_W, CELL_H), 0)
        d = ImageDraw.Draw(im)
        d.fontmode = "1"                     # no anti-aliasing
        d.text((0, 0), ch, font=_font, fill=1)
        rows = ["".join("1" if im.getpixel((x, y)) else "." for x in range(CELL_W))
                for y in range(TOP, TOP + LINES)]
        self[ch] = rows
        return rows


FONT = Glyphs()

grid = {}  # (x, y) -> colour


def put(x, y, c):
    if 0 <= x < W and 0 <= y < H:
        grid[(x, y)] = c


# background with the horizontal stripes of the raster
for y in range(H):
    for x in range(W):
        put(x, y, BG)
y_stripe = 18 + 4 * 2          # behind the middle of the letters
for i, c in enumerate(STRIPES):
    for y in range(y_stripe + i * 4, y_stripe + i * 4 + 4):
        for x in range(W):
            put(x, y, c)

# the big word: the cells of the glyphs side by side (the 9th column is the gap)
word = "RITMO"
gw = CELL_W * BIG
total = len(word) * gw - BIG
x0 = (W - total) // 2
y0 = 14
letter = set()
for n, ch in enumerate(word):
    ox = x0 + n * gw
    for gy, row in enumerate(FONT[ch]):
        for gx, v in enumerate(row):
            if v == "1":
                for dy in range(BIG):
                    for dx in range(BIG):
                        letter.add((ox + gx * BIG + dx, y0 + gy * BIG + dy))

# shadow, outline, then the letters with a colour per raster line
for (x, y) in letter:
    put(x + 2, y + 2, SHADOW)
outline = set()
for (x, y) in letter:
    for dx in (-1, 0, 1):
        for dy in (-1, 0, 1):
            if (x + dx, y + dy) not in letter:
                outline.add((x + dx, y + dy))
for (x, y) in outline:
    put(x, y, OUTLINE)
hh = LINES * BIG
for (x, y) in letter:
    put(x, y, BARS[(y - y0) * len(BARS) // hh])

# the subtitle
sub = "ATARI POKEY MUSIC TRACKER"
sw = len(sub) * CELL_W - 1
sx = (W - sw) // 2
sy = y0 + hh + 10
for n, ch in enumerate(sub):
    for gy, row in enumerate(FONT[ch]):
        for gx, v in enumerate(row):
            if v == "1":
                put(sx + n * CELL_W + gx + 1, sy + gy + 1, OUTLINE)
                put(sx + n * CELL_W + gx, sy + gy, SUB)

# the pattern of a tracker: the line number and three tracks of "note instrument
# volume", the current line on a bar
PATTERN = [
    ("0C", ["C-2 01 F", "--- -- -", "E-3 04 B"]),
    ("0D", ["--- -- -", "E-4 05 C", "--- -- -"]),
    ("0E", ["G-2 01 D", "--- -- -", "G-4 02 A"]),
    ("0F", ["--- -- -", "D-4 05 A", "--- -- -"]),
    ("10", ["C-2 01 F", "E-4 05 8", "C-5 02 F"]),
]
CURRENT = 2
PITCH = LINES + 2
py = sy + LINES + 10
PANEL, PANEL_EDGE, CURSOR_BAR = "#0f0f38", "#3a3a8a", "#2c2c7c"
for x in range(6, W - 6):
    for y in range(py - 4, py + len(PATTERN) * PITCH + 2):
        edge = x in (6, W - 7) or y in (py - 4, py + len(PATTERN) * PITCH + 1)
        put(x, y, PANEL_EDGE if edge else PANEL)
for x in range(8, W - 8):
    for y in range(py + CURRENT * PITCH - 1, py + CURRENT * PITCH + LINES + 1):
        put(x, y, CURSOR_BAR)


def text(s, x, y, c):
    for n, ch in enumerate(s):
        for gy, row in enumerate(FONT[ch]):
            for gx, v in enumerate(row):
                if v == "1":
                    put(x + n * CELL_W + gx, y + gy, c)


px0 = (W - 29 * CELL_W) // 2
for r, (num, tracks) in enumerate(PATTERN):
    y = py + r * PITCH
    now = r == CURRENT
    text(num, px0, y, "#ffffff" if now else "#7878b8")
    for t, cell in enumerate(tracks):
        x = px0 + (3 + t * 9) * CELL_W
        note, inst, vol = cell[:3], cell[4:6], cell[7]
        text(note, x, y, "#8fd0ff" if note[0] != "-" else "#4a4a8a")
        text(inst, x + 4 * CELL_W, y, "#ffd23c" if inst[0] != "-" else "#4a4a8a")
        text(vol, x + 7 * CELL_W, y, "#7cf07c" if vol != "-" else "#4a4a8a")

# the lines at the top and at the bottom
for x in range(W):
    put(x, 3, ACCENT)
    put(x, 4, "#ffd23c")
    put(x, H - 5, "#ffd23c")
    put(x, H - 4, ACCENT)

# ------------------------------------------------------------------ output
# PNG
img = Image.new("RGB", (W * SCALE, H * SCALE))
px = img.load()
for (x, y), c in grid.items():
    r, g, b = int(c[1:3], 16), int(c[3:5], 16), int(c[5:7], 16)
    for dy in range(SCALE):
        for dx in range(SCALE):
            px[x * SCALE + dx, y * SCALE + dy] = (r, g, b)
img.save("ritmo-logo.png")

# SVG: runs of equal colour in a row
out = [f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {W} {H}" width="{W * SCALE}" height="{H * SCALE}" '
       f'shape-rendering="crispEdges">', "<title>RITMO - Atari POKEY music tracker</title>"]
for y in range(H):
    x = 0
    while x < W:
        c = grid[(x, y)]
        n = 1
        while x + n < W and grid[(x + n, y)] == c:
            n += 1
        out.append(f'<rect x="{x}" y="{y}" width="{n}" height="1" fill="{c}"/>')
        x += n
out.append("</svg>")
open("ritmo-logo.svg", "w").write("\n".join(out) + "\n")
print("ritmo-logo.png", img.size, "ritmo-logo.svg", len(out) - 3, "rects")
