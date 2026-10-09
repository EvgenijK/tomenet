#!/usr/bin/env python3
"""Independent Net_start oracle for the SV text-only default resource outcome.

Reads legacy option_info + shipped options.prf, never an SV serializer.
Cascadia has no font-custom mapping and no font-sdl3.prf exists: baseline
handle_process_font_file clears all four complete redefinition arrays. Zero
entries mean retain server identity, not an abbreviated test fixture.
"""
from pathlib import Path
import re
import struct
import subprocess
ROOT = Path(__file__).resolve().parents[3]
source = (ROOT / 'src/client/c-tables.c').read_text()
section = source[source.index('option_type option_info'):]
section = section[:section.index('\n};') + 3]
section = subprocess.check_output(['clang', '-E', '-P', '-x', 'c', '-DGLOBAL_BIG_MAP', '-'], input=section, text=True)
rows = re.findall(r'\{\s*&c_cfg\.\w+,\s*(TRUE|FALSE),.*?"([^"]+)"', section, re.S)
assert len(rows) == 188
options = bytearray(199)
names = {}
for i, (default, name) in enumerate(rows):
    options[i] = default == 'TRUE'
    names[name] = i
for line in (ROOT / 'lib/user/options.prf').read_text().splitlines():
    if line.startswith(('X:', 'Y:')) and line[2:] in names:
        options[names[line[2:]]] = line[0] == 'Y'
# Native flow: male, Mage, Dwarf, Phoenix, fruit bat, Normal; stat order a..f.
packet = b'\x03' + struct.pack('!7h', 0x300, 2, 2, 2, 0, 0, 0)
packet += b'NO_GRAPHICS\0CascadiaMono-Regular.ttf\0'
packet += struct.pack('!6h', 0, 1, 2, 3, 4, 5) + options
packet += struct.pack('!2i', 66, 22)
packet += bytes(5 * (127 + 512 + 1280 + 1280))
Path(__file__).with_name('birth-current.hex').write_text(packet.hex() + '\n')
print(f'Baseline handshake: {len(packet)} bytes; {sum(options)} enabled options')
