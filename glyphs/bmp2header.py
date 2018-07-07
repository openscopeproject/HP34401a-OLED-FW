from PIL import Image
from glob import glob

h = open('glyphs.h', 'w')

h.write("""\
#ifndef __GLYPHS_H
#define __GLYPHS_H
#include "Arduino.h"

struct Glyph {
  uint8_t width, height;
  uint8_t data[];
};

""")


def convert_slice(l):
  while len(l) < 8:
    l.append(1)
  return reduce(lambda a, v: a * 2 + (0 if v > 0 else 1), l, 0)


def to_bin(b):
  return format(b, '08b').replace('0', '-').replace('1', '#')


def process_file(name):
  im = Image.open(name + ".bmp")
  image_data = list(im.getdata())
  width = im.size[0]
  height = im.size[1]
  bytes_per_row = (width + 7) // 8
  h.write('const Glyph {} = {{\n'.format(name))
  h.write('  {}, {},\n'.format(width, height))
  h.write('  {\n')
  for i in xrange(height):
    row = image_data[width * i:width * (i + 1)]
    data_bytes = [convert_slice(row[8 * i:8 * i + 8]) for i in
                  xrange(bytes_per_row)]
    data_bytes_hex = ['0x%02x' % b for b in data_bytes]
    data_bytes_bin = [to_bin(b) for b in data_bytes]
    h.write('    ' + ', '.join(data_bytes_hex) + ', ')
    h.write('// ' + ''.join(data_bytes_bin) + '\n')
  h.write('  }\n')
  h.write('};\n\n')


bmp_files = glob('*.bmp')
for f in bmp_files:
  process_file(f[:-4])

h.write("""\
#endif
""")
