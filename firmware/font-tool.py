#!/usr/bin/env python3

import sys
import string
import PIL
from PIL import ImageFont, Image


PIXEL_HEIGHT = 10
LETTERS = "0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz"


if len(sys.argv) != 4:
    print("Usage: %s font_path font_size output_path" % sys.argv[0])
    exit(1)


# load font
font_path = sys.argv[1]
font_size = int(sys.argv[2])
output_path = sys.argv[3]
out = open(output_path, "w")

font = ImageFont.FreeTypeFont(font_path, size=font_size)


# check font size
if font.getsize(LETTERS)[1] > 10:
    print("Font size is too large.")
    exit(1)


# check if it is a monospace font
size = font.getsize(LETTERS[0])[0]
for l in LETTERS:
    if font.getsize(l)[0] != size:
        print("Not a monospace font!")
        exit(1)

out.write("#include <avr/pgmspace.h>\n")
out.write("#define FONT_CHAR_WIDTH %d\n" % size)


# render font to image (L = render luminance aka. white on black)
mask = font.getmask(LETTERS, "L")

# "normalize" it
for x in range(mask.size[0]):
    for y in range(mask.size[1]):
        if mask.getpixel((x, y)) > 127:
            mask.putpixel((x, y), 1)
        else:
            mask.putpixel((x, y), 0)


# top two lines first
top = []
current = 0x00
cnt = 0
for x in range(mask.size[0]):
    col = (mask.getpixel((x, 0)) << 1) | (mask.getpixel((x, 1)) << 0)
    current |= col << (cnt << 1)
    cnt += 1

    if cnt == 4:
        top.append(current)
        current = 0x00
        cnt = 0


# bottom 8 lines
bottom = []
for x in range(mask.size[0]):
    col = 0x00
    for y in range(8):
        col <<= 1
        col |= mask.getpixel((x, y))

    bottom.append(col)


# now write top and bottom lines to file
top_s = "const char font_top[] PROGMEM = {%s};\n"
bottom_s = "const char font_bottom[] PROGMEM = {%s};\n"


def bytes_to_c_str(b):
    return ", ".join((("0x%02x" % c) for c in b))


top_s %= bytes_to_c_str(top)
bottom_s %= bytes_to_c_str(bottom)

out.write("\n")
out.write(top_s)
out.write(bottom_s)

out.close()
