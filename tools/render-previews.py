#!/usr/bin/env python3
"""Render UI trace files from test/ui_preview.cpp with the VT323 1-bit bitmaps."""
from pathlib import Path
import re, zlib, struct, sys, math

root = Path(__file__).resolve().parents[1]
output = Path(sys.argv[1])
scale = int(sys.argv[2]) if len(sys.argv) > 2 else 1
width, height = 240, 135
text = (root / "src/assets/UiFonts.h").read_text()
fonts = {}
for size in (16, 17, 18, 19, 20, 21, 22, 23, 31):
    name = f"Vt323{size}"
    block = text.split(f"const uint8_t {name}Bitmaps")[1].split(f"const lgfx::GFXfont {name} ")[0]
    raw = block.split("= {", 1)[1].split("};", 1)[0]
    bits = bytes(int(value, 16) for value in re.findall(r"0x([0-9A-Fa-f]{2})", raw))
    glyphs = {}
    for match in re.findall(
        r"\{\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*(-?\d+),\s*(-?\d+)\s*\}[^\n]*// 0x([0-9A-Fa-f]+)",
        block,
    ):
        glyphs[int(match[6], 16)] = tuple(map(int, match[:6]))
    fonts[size] = (bits, glyphs, max(-glyph[5] for glyph in glyphs.values()))

def rgb(color):
    return bytes((((color >> 11) & 31) * 255 // 31, ((color >> 5) & 63) * 255 // 63, (color & 31) * 255 // 31))

def png(path, pixels, image_width, image_height):
    def chunk(tag, data):
        return struct.pack("!I", len(data)) + tag + data + struct.pack("!I", zlib.crc32(tag + data) & 0xFFFFFFFF)
    rows = b"".join(b"\0" + pixels[y * image_width * 3:(y + 1) * image_width * 3] for y in range(image_height))
    path.write_bytes(
        b"\x89PNG\r\n\x1a\n"
        + chunk(b"IHDR", struct.pack("!IIBBBBB", image_width, image_height, 8, 2, 0, 0, 0))
        + chunk(b"IDAT", zlib.compress(rows))
        + chunk(b"IEND", b"")
    )

for source in output.glob("*.ops"):
    pixels = bytearray(width * height * 3)
    def put(x, y, color):
        if 0 <= x < width and 0 <= y < height:
            pixels[(y * width + x) * 3:(y * width + x) * 3 + 3] = color
    for op in source.read_text().splitlines():
        if op.startswith("F "):
            _, x, y, w, h, color = op.split()
            x, y, w, h, color = map(int, (x, y, w, h, color))
            col = rgb(color)
            for row in range(y, y + h):
                for column in range(x, x + w):
                    put(column, row, col)
        elif op.startswith("T "):
            _, x, y, kind, color, _, glyph_text = op.split(" ", 6)
            x, y, kind, color = map(int, (x, y, kind, color))
            bits, glyphs, baseline = fonts[kind]
            col = rgb(color)
            cursor = x
            for char in glyph_text:
                glyph = glyphs.get(ord(char))
                if not glyph:
                    cursor += kind // 2
                    continue
                offset, gw, gh, advance, x_offset, y_offset = glyph
                for row in range(gh):
                    for column in range(gw):
                        bit = row * gw + column
                        if bits[offset + bit // 8] & (0x80 >> (bit % 8)):
                            put(cursor + x_offset + column, y + baseline + y_offset + row, col)
                cursor += advance
    if scale == 1:
        png(source.with_suffix(".png"), bytes(pixels), width, height)
        continue
    rows = []
    for y in range(height):
        row = b"".join(bytes(pixels[(y * width + x) * 3:(y * width + x) * 3 + 3]) * scale for x in range(width))
        rows.extend([row] * scale)
    png(source.with_suffix(".png"), b"".join(rows), width * scale, height * scale)
print(f"VT323 previews rendered at {width}x{height}.")
