#!/usr/bin/env python3
"""Checks that the WAV export of a song lasts as long as its SAP-R stream: the stream has one frame per call of the
tracker driver (FASTPLAY lines of 114 cycles each, a song at instrument speed 4 has four frames per VBI) and the WAV
export renders 1/instrument speed of a VBI for each (an earlier version rendered a whole VBI for each frame, so the
WAV of a song at instrument speed 4 lasted four times as long).

    scripts/check-wav.py file.sapr file.wav
"""
import struct
import sys

sapr = open(sys.argv[1], "rb").read()
end = sapr.find(b"\r\n\r\n")
header = sapr[:end].decode("latin-1")
tags = dict(line.partition(" ")[::2] for line in header.split("\r\n")[1:])
stereo = "STEREO" in header.split("\r\n")
ntsc = "NTSC" in header.split("\r\n")
frames = (len(sapr) - end - 4) // (18 if stereo else 9)
lines, hertz = (262, 60) if ntsc else (312, 50)
fastplay = int(tags.get("FASTPLAY", lines))
expected = frames * fastplay / (hertz * lines)

wav = open(sys.argv[2], "rb").read(44)
channels, rate, _, _, bits = struct.unpack("<HIIHH", wav[22:36])
size = struct.unpack("<I", wav[40:44])[0]
seconds = size / (rate * channels * bits // 8)

ok = abs(seconds - expected) <= expected * 0.002
print("%s %s: the stream %d frames = %.2f s, the WAV %.2f s" % ("ok  " if ok else "FAIL", sys.argv[2].rsplit("/", 1)[-1], frames, expected, seconds))
sys.exit(0 if ok else 1)
