#!/usr/bin/env python3
"""Dedicated literal outcomes applied to the independent ordinary baseline oracle.

The resource/default option portion is unchanged by dedicated creation in
legacy get_char_info/Net_start. The base fixture is produced independently from
c-tables.c and options.prf by birth-current-baseline.py, never from SV bytes.
"""
from pathlib import Path
import struct

HERE = Path(__file__).resolve().parent
base = bytes.fromhex((HERE / "birth-current.hex").read_text())
assert len(base) == 16266
assert base[:9] == bytes.fromhex("03 0300 0002 0002 0002")
# c-birth.c choose_mode: MODE_MALE | MODE_FRUIT_BAT and
# MODE_NO_GHOST | MODE_DED_IDDC, adding SOLO or HARD where selected.
# get_char_info: skipped trait (-1) becomes wire trait 0. For PvP Maia,
# choose_trait adds Enlightened (14)/Corrupted (15) after choose_mode;
# BACKSPACE returns to cstats, whose newly committed order reaches Net_start.
scenarios = [
    ("iddc", 0x344, 2, 2, 2, (0, 1, 2, 3, 4, 5)),
    ("iddc-soloist", 0x345, 2, 2, 2, (0, 1, 2, 3, 4, 5)),
    ("iddc-hellish", 0x346, 2, 2, 2, (0, 1, 2, 3, 4, 5)),
    ("iddc-skipped", 0x344, 2, 2, 0, (0, 1, 2, 3, 4, 5)),
    ("pvp-maia", 0x390, 17, 0, 15, (5, 4, 3, 2, 1, 0)),
]
stat_offset = 15 + len(b"NO_GRAPHICS\0CascadiaMono-Regular.ttf\0")
for name, mode, race, class_id, trait, stats in scenarios:
    packet = bytearray(base)
    packet[1:9] = struct.pack("!4h", mode, race, class_id, trait)
    packet[stat_offset:stat_offset + 12] = struct.pack("!6h", *stats)
    (HERE / f"birth-{name}.hex").write_text(packet.hex() + "\n")
    print(f"{name}: {len(packet)} independent baseline bytes")
