#!/usr/bin/env python3
"""Render UI trace files from test/ui_preview.cpp using the pinned font bitmaps."""
from pathlib import Path
import re,zlib,struct,sys,math
root=Path(__file__).resolve().parents[1]
output=Path(sys.argv[1])
fontroot=root / Path('.pio/libdeps/m5stickc-plus2/M5GFX/src/lgfx/Fonts/GFXFF')
fonts={}
for kind,name in [(0,'FreeMono9pt7b'),(1,'FreeMonoBold9pt7b'),(2,'FreeMonoBold12pt7b'),(3,'FreeMonoCompact14'),(4,'FreeMonoBody16')]:
 s=(fontroot/(name+'.h')).read_text() if kind<3 else (root/'src/assets/UiFonts.h').read_text().split('const uint8_t '+name+'Bitmaps')[1].split('const lgfx::GFXfont '+name)[0]
 if kind>=3:s='Bitmaps'+s
 raw=re.split(r'Bitmaps\[\](?: PROGMEM)? = \{',s,maxsplit=1)[1].split('};',1)[0]
 bits=bytes(int(x,16) for x in re.findall(r'0x([0-9A-Fa-f]{2})',raw))
 glyphs={int(m[6],16):tuple(map(int,m[:6])) for m in re.findall(r'\{\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+),\s*(-?\d+),\s*(-?\d+)\s*\}[^\n]*// 0x([0-9A-Fa-f]+)',s)}
 fonts[kind]=(bits,glyphs,max(-g[5] for g in glyphs.values()))
def rgb(c):return bytes((((c>>11)&31)*255//31,((c>>5)&63)*255//63,(c&31)*255//31))
def png(path,pixels,w,h):
 def chunk(tag,data):return struct.pack('!I',len(data))+tag+data+struct.pack('!I',zlib.crc32(tag+data)&0xffffffff)
 data=b''.join(b'\0'+pixels[y*w*3:(y+1)*w*3] for y in range(h))
 path.write_bytes(b'\x89PNG\r\n\x1a\n'+chunk(b'IHDR',struct.pack('!IIBBBBB',w,h,8,2,0,0,0))+chunk(b'IDAT',zlib.compress(data))+chunk(b'IEND',b''))
for source in output.glob('*.ops'):
 pixels=bytearray(135*240*3)
 def put(x,y,c):
  if 0<=x<135 and 0<=y<240:pixels[(y*135+x)*3:(y*135+x)*3+3]=c
 for op in source.read_text().splitlines():
  if op.startswith('F '):
   _,x,y,w,h,c=op.split();x,y,w,h,c=map(int,(x,y,w,h,c));col=rgb(c)
   for row in range(y,y+h):
    for xx in range(x,x+w):put(xx,row,col)
  else:
   _,x,y,kind,c,scale,text=op.split(' ',6);x,y,kind,c=map(int,(x,y,kind,c));scale=float(scale);col=rgb(c)
   bits,glyphs,baseline=fonts[kind]
   cursor=float(x)
   for char in text:
    off,w,h,advance,xoff,yoff=glyphs[ord(char)]
    # Nearest-neighbor approximation of M5GFX's fractional FreeMono scaling.
    for row in range(math.ceil((baseline+yoff)*scale),math.ceil((baseline+yoff+h)*scale)):
     sy=math.floor(row/scale)-baseline-yoff
     for xx in range(math.ceil(xoff*scale),math.ceil((xoff+w)*scale)):
      sx=math.floor(xx/scale)-xoff
      bit=sy*w+sx
      if 0<=sx<w and 0<=sy<h and bits[off+bit//8]&(0x80>>(bit%8)):put(int(cursor)+xx,y+row,col)
    cursor+=advance*scale
 rows=[]
 for y in range(240):
  row=b''.join(bytes(pixels[(y*135+x)*3:(y*135+x)*3+3])*4 for x in range(135));rows.extend([row]*4)
 png(source.with_suffix('.png'),b''.join(rows),540,960)
print('Eleven FreeMono/cream previews rendered from the current UI.')
