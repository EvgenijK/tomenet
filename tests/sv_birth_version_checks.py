#!/usr/bin/env python3
"""Baseline gate examples; actual bytes cross sv_endpoint_run/native SDL/TCP.

Rows specify layouts independently of the production serializer. The literal
option outcome is the baseline-derived current fixture, never SV source/output.
"""
from pathlib import Path
import json
import os
import runpy
import tempfile

ROOT = Path(__file__).resolve().parents[1]
# version, identity layout, options, geometry, character bytes, F/K/R counts
# Each row is explicit: below / at / above a baseline comparison. Neighbor
# tuples are controlled protocol probes, not claims of historical releases.
ROWS = [
    # Net_setup class recommendation bytes (> 4.4.3.1.0.0)
    ("4.4.3.0.0.0", "original", 96, False, 1, 256, 1024, 1152),
    ("4.4.3.1.0.0", "original", 96, False, 1, 256, 1024, 1152),
    ("4.4.3.1.0.1", "original", 96, False, 1, 256, 1024, 1152),
    # Net_setup trait count and Net_start trait/audio (> 4.4.5.10.0.0)
    ("4.4.5.9.0.0", "original", 96, False, 1, 256, 1024, 1152),
    ("4.4.5.10.0.0", "original", 96, False, 1, 256, 1024, 1152),
    ("4.4.5.10.0.1", "audio", 96, False, 1, 256, 1024, 1152),
    # Geometry (> 4.4.9.1.0.1)
    ("4.4.9.1.0.0", "audio", 96, False, 1, 256, 1024, 1152),
    ("4.4.9.1.0.1", "audio", 96, False, 1, 256, 1024, 1152),
    ("4.4.9.1.0.2", "audio", 96, True, 1, 256, 1024, 1152),
    # Character overview mode (> 4.4.9.2.0.0)
    ("4.4.9.1.0.3", "audio", 96, True, 1, 256, 1024, 1152),
    ("4.4.9.2.0.0", "audio", 96, True, 1, 256, 1024, 1152),
    ("4.4.9.2.0.1", "audio", 96, True, 1, 256, 1024, 1152),
    # 128 options (> 4.5.5.0.0.0)
    ("4.5.4.0.0.0", "audio", 96, True, 1, 256, 1024, 1152),
    ("4.5.5.0.0.0", "audio", 96, True, 1, 256, 1024, 1152),
    ("4.5.5.0.0.1", "audio", 128, True, 1, 256, 1024, 1152),
    # Character overview location (> 4.5.7.0.0.0)
    ("4.5.6.0.0.0", "audio", 128, True, 1, 256, 1024, 1152),
    ("4.5.7.0.0.0", "audio", 128, True, 1, 256, 1024, 1152),
    ("4.5.7.0.0.1", "audio", 128, True, 1, 256, 1024, 1152),
    # 154 options (> 4.5.8.1.0.1)
    ("4.5.8.1.0.0", "audio", 128, True, 1, 256, 1024, 1152),
    ("4.5.8.1.0.1", "audio", 128, True, 1, 256, 1024, 1152),
    ("4.5.8.1.0.2", "audio", 154, True, 1, 256, 1024, 1152),
    # K/R expansion; F remains 256 (> 4.6.1.2.0.0)
    ("4.6.1.1.0.0", "audio", 154, True, 1, 256, 1024, 1152),
    ("4.6.1.2.0.0", "audio", 154, True, 1, 256, 1024, 1152),
    ("4.6.1.2.0.1", "audio", 154, True, 1, 256, 1280, 1280),
    # u32 characters (>= 4.8.1.0.0.0)
    ("4.8.0.0.0.0", "audio", 154, True, 1, 256, 1280, 1280),
    ("4.8.1.0.0.0", "audio", 154, True, 4, 256, 1280, 1280),
    ("4.8.1.0.0.1", "audio", 154, True, 4, 256, 1280, 1280),
    # Graphics/font/resources (>= 4.8.1.2.0.0)
    ("4.8.1.1.0.0", "audio", 154, True, 4, 256, 1280, 1280),
    ("4.8.1.2.0.0", "resources", 154, True, 4, 256, 1280, 1280),
    ("4.8.1.2.0.1", "resources", 154, True, 4, 256, 1280, 1280),
    # 199 options (> 4.9.1.0.0.0)
    ("4.9.0.0.0.0", "resources", 154, True, 4, 256, 1280, 1280),
    ("4.9.1.0.0.0", "resources", 154, True, 4, 256, 1280, 1280),
    ("4.9.1.0.0.1", "resources", 199, True, 4, 256, 1280, 1280),
    # F expansion (> 4.9.1.2.0.2), despite stale defines.h commentary
    ("4.9.1.2.0.1", "resources", 199, True, 4, 256, 1280, 1280),
    ("4.9.1.2.0.2", "resources", 199, True, 4, 256, 1280, 1280),
    ("4.9.1.2.0.3", "resources", 199, True, 4, 512, 1280, 1280),
    # Initial login identity address (>= 4.9.2.1.0.2)
    ("4.9.2.1.0.1", "resources", 199, True, 4, 512, 1280, 1280),
    ("4.9.2.1.0.2", "resources", 199, True, 4, 512, 1280, 1280),
    ("4.9.2.1.0.3", "resources", 199, True, 4, 512, 1280, 1280),
    ("4.9.4.0.0.0", "resources", 199, True, 4, 512, 1280, 1280),
]
with tempfile.TemporaryDirectory(prefix="sv-version-oracles-") as temp:
    directory = Path(temp)
    current = bytes.fromhex((ROOT / "tests/sv/fixtures/birth-current.hex").read_text())
    # Baseline fixture's option section begins at 64; keep its literal defaults.
    options = current[64:263]
    assert len(options) == 199 and sum(options) == 88
    rows = []
    selected = os.environ.get("SV_BIRTH_VERSION")
    for version, identity, count, geometry, width, features, kinds, monsters in ROWS:
        if selected and version != selected:
            continue
        # Known baseline example: male + fruit bat Normal, Dwarf, Mage,
        # Phoenix where traits exist; the same six committed stat ranks.
        packet = bytes.fromhex("03 0300 0002 0002")
        if identity != "original":
            packet += bytes.fromhex("0002 0000 0000")
        if identity == "resources":
            packet += bytes.fromhex("0000") + b"NO_GRAPHICS\0CascadiaMono-Regular.ttf\0"
        packet += bytes.fromhex("0000 0001 0002 0003 0004 0005")
        packet += options[:count]
        if geometry:
            packet += bytes.fromhex("00000042 00000016")
        packet += bytes((1 + width) * (127 + features + kinds + monsters))
        fixture = directory / (version + ".hex")
        fixture.write_text(packet.hex())
        rows.append({"version": version, "fixture": str(fixture)})
    assert rows, "No selected baseline example"
    matrix = directory / "matrix.json"
    matrix.write_text(json.dumps(rows))
    os.environ["SV_BIRTH_MATRIX"] = str(matrix)
    runpy.run_path(str(ROOT / "tests/sv_birth_complete_checks.py"), run_name="__main__")
