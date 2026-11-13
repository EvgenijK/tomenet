#!/usr/bin/env python3
"""Snapshot original prototype SVG geometry and compile vector commands for SV.

Authoring uses the Python standard library. The client uses its existing FreeType dependency.
Usage: python3 generate.py /path/to/tomenet_interface
This is vector rasterization, not a hand-drawn approximation of the SVG paths.
"""
from pathlib import Path
import hashlib
import json
import re
import math
import sys
import xml.etree.ElementTree as ET

HERE = Path(__file__).resolve().parent
PROTOTYPE = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else None
SOURCE_FORMAT = "svg-stroke-commands-v1"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def snapshot():
    if PROTOTYPE is None:
        return json.loads((HERE / 'sources.json').read_text())
    html = (PROTOTYPE / 'index.html').read_text()
    data = (PROTOTYPE / 'js/data/game-data.js').read_text()
    items = []

    def add(name, svg, source, selector, stroke):
        filename = name.lower().replace('_', '-') + '.svg'
        # The paths/primitive attributes remain byte-identical to the source.
        # Only inherited external CSS is made explicit on the SVG root.
        normalized = svg.replace('<svg ', '<svg xmlns="http://www.w3.org/2000/svg" ', 1)
        normalized = normalized.replace('><', f' fill="none" stroke="#fff" stroke-width="{stroke}" stroke-linecap="round" stroke-linejoin="round"><', 1)
        (HERE / filename).write_text(normalized + '\n')
        items.append(dict(name=name, file=filename, source=source, selector=selector,
                          stroke_width=stroke, source_svg_sha256=digest(svg.encode()),
                          snapshot_sha256=digest((normalized + '\n').encode())))

    attributes = ['strength', 'intelligence', 'wisdom', 'dexterity', 'constitution', 'charisma']
    for name in attributes:
        match = re.search(r'class="stat-icon ' + name + r'-icon"[^>]*>(<svg.*?</svg>)', html)
        assert match, name
        add('ATTR_' + name.upper(), match[1], 'index.html', '.stat-icon.' + name + '-icon svg',
            '1.35' if name == 'constitution' else '1.25' if name == 'charisma' else '1.45')
    for name, cls in [('SANITY', 'sanity-icon'), ('ARMOUR', 'shield-icon')]:
        match = re.search(r'class="line-icon ' + cls + r'"[^>]*>(<svg.*?</svg>)', html)
        add(name, match[1], 'index.html', '.line-icon.' + cls + ' svg', '1.4')
    match = re.search(r'id="characterShortcut"[^>]*>\s*(<svg.*?</svg>)', html)
    add('CHARACTER', match[1], 'index.html', '#characterShortcut svg', '1.45')
    block = data.split('const ENCUMBRANCE_STATUSES = [', 1)[1].split('\n  ];', 1)[0]
    for match in re.finditer(r'\{id:"(\w+)"[^\n]*?icon:\'(<svg.*?</svg>)\'', block):
        add('ENC_' + match[1].upper(), match[2], 'js/data/game-data.js',
            'ENCUMBRANCE_STATUSES.' + match[1] + '.icon', '1.45')
    assert sum(x['name'].startswith('ENC_') for x in items) == 15
    block = data.split('const STATUS_ICONS = {', 1)[1].split('\n  };', 1)[0]
    for match in re.finditer(r'(\w+):\'(<svg.*?</svg>)\'', block):
        add('STATUS_' + match[1].upper(), match[2], 'js/data/game-data.js', 'STATUS_ICONS.' + match[1], '1.45')
    assert sum(x['name'].startswith('STATUS_') for x in items) == 29
    block = data.split('const CHARACTER_SOURCE_ICONS = {', 1)[1].split('\n  };', 1)[0]
    for match in re.finditer(r'(\w+):\'(<svg.*?</svg>)\'', block):
        add('SOURCE_' + match[1].upper(), match[2], 'js/data/game-data.js',
            'CHARACTER_SOURCE_ICONS.' + match[1], '1.45')
    source_files = ['index.html', 'js/data/game-data.js', 'css/hud.css', 'css/shortcuts.css']
    manifest = dict(prototype_root=str(PROTOTYPE), source_files_sha256={f: digest((PROTOTYPE / f).read_bytes()) for f in source_files},
                    vector_format=SOURCE_FORMAT,
                    icons=items)
    (HERE / 'sources.json').write_text(json.dumps(manifest, indent=2) + '\n')
    return manifest


def arc_segments(start, rx, ry, rotation, large, sweep, end):
    """SVG endpoint arc conversion, split at <=90 degrees into cubic segments."""
    if start == end:
        return []
    rx, ry = abs(rx), abs(ry)
    if not rx or not ry:
        return [('L', *end)]
    phi = math.radians(rotation % 360)
    cp, sp = math.cos(phi), math.sin(phi)
    dx, dy = (start[0] - end[0]) / 2, (start[1] - end[1]) / 2
    xp, yp = cp * dx + sp * dy, -sp * dx + cp * dy
    radius = xp*xp/(rx*rx) + yp*yp/(ry*ry)
    if radius > 1:
        rx *= math.sqrt(radius)
        ry *= math.sqrt(radius)
    numerator = max(0, rx*rx*ry*ry - rx*rx*yp*yp - ry*ry*xp*xp)
    denominator = rx*rx*yp*yp + ry*ry*xp*xp
    factor = (-1 if bool(large) == bool(sweep) else 1) * math.sqrt(numerator / denominator) if denominator else 0
    cxp, cyp = factor * rx * yp / ry, -factor * ry * xp / rx
    cx = cp*cxp - sp*cyp + (start[0]+end[0])/2
    cy = sp*cxp + cp*cyp + (start[1]+end[1])/2
    ux, uy = (xp-cxp)/rx, (yp-cyp)/ry
    vx, vy = (-xp-cxp)/rx, (-yp-cyp)/ry
    theta = math.atan2(uy, ux)
    delta = math.atan2(ux*vy-uy*vx, ux*vx+uy*vy)
    if not sweep and delta > 0:
        delta -= 2*math.pi
    elif sweep and delta < 0:
        delta += 2*math.pi
    count = max(1, math.ceil(abs(delta)/(math.pi/2)))
    step = delta/count
    def point(a):
        return (cx+rx*cp*math.cos(a)-ry*sp*math.sin(a), cy+rx*sp*math.cos(a)+ry*cp*math.sin(a))
    def tangent(a):
        return (-rx*cp*math.sin(a)-ry*sp*math.cos(a), -rx*sp*math.sin(a)+ry*cp*math.cos(a))
    output = []
    for i in range(count):
        a, b = theta+i*step, theta+(i+1)*step
        first, last, ta, tb = point(a), point(b), tangent(a), tangent(b)
        k = 4/3*math.tan(step/4)
        output.append(('C', first[0]+k*ta[0], first[1]+k*ta[1], last[0]-k*tb[0], last[1]-k*tb[1], *last))
    output[-1] = (*output[-1][:-2], *end)
    return output


def path_commands(data):
    tokens = re.findall(r'[A-Za-z]|[-+]?(?:\d*\.\d+|\d+\.?\d*)(?:[eE][-+]?\d+)?', data)
    commands, contour, current, start, control = [], [], (0., 0.), (0., 0.), None
    pos, operation, previous = 0, None, None
    def finish(closed):
        nonlocal contour
        if contour:
            commands.append(('M', *contour[0][1:], 1 if closed else 0))
            commands.extend(contour[1:])
            commands.append(('E',))
            contour = []
    while pos < len(tokens):
        if tokens[pos].isalpha():
            operation = tokens[pos]
            pos += 1
        upper = operation.upper()
        relative = operation.islower()
        if upper == 'Z':
            finish(True)
            current, control, previous, operation = start, None, 'Z', None
            continue
        count = {'M':2, 'L':2, 'H':1, 'V':1, 'C':6, 'S':4, 'A':7}[upper]
        values = list(map(float, tokens[pos:pos+count]))
        assert len(values) == count
        pos += count
        origin = current
        def xy(x, y):
            return (x+origin[0], y+origin[1]) if relative else (x,y)
        if upper == 'M':
            finish(False)
            current = start = xy(*values)
            contour.append(('M', *current))
            operation = 'l' if relative else 'L'
            control = None
        elif upper in ('L','H','V'):
            current = xy(*values) if upper == 'L' else ((values[0]+origin[0] if relative else values[0]), origin[1]) if upper == 'H' else (origin[0], (values[0]+origin[1] if relative else values[0]))
            contour.append(('L', *current))
            control = None
        elif upper in ('C','S'):
            first = xy(*values[:2]) if upper == 'C' else (2*origin[0]-control[0], 2*origin[1]-control[1]) if previous in ('C','S') and control else origin
            second = xy(*values[-4:-2])
            current = xy(*values[-2:])
            contour.append(('C', *first, *second, *current))
            control = second
        elif upper == 'A':
            current = xy(*values[-2:])
            contour.extend(arc_segments(origin, *values[:5], current))
            control = None
        previous = upper
    finish(False)
    return commands


def compile_icons(manifest):
    header = HERE.parent / 'hud-icons.h'
    # API is maintained in hud-icons.h; generation does not overwrite it.
    names = re.findall(r'SV_HUD_ICON_([A-Z_]+),', header.read_text())
    assert names == [x['name'] for x in manifest['icons']]
    output = ['/* Generated SVG stroke commands. No pre-rasterized icon masks. */']
    primitives, geometries, offset = [], [], 0
    generation = hashlib.sha256(''.join(x['snapshot_sha256'] for x in manifest['icons']).encode()).hexdigest()[:16]
    for index, item in enumerate(manifest['icons']):
        svg = (HERE / item['file']).read_bytes()
        assert digest(svg) == item['snapshot_sha256'], item['file']
        element = ET.fromstring(svg)
        view = list(map(float, element.attrib['viewBox'].split()))
        assert view[:2] == [0,0]
        first_primitive = len(primitives)
        for primitive in element:
            tag = primitive.tag.split('}')[-1]
            if tag == 'path':
                commands = path_commands(primitive.attrib['d'])
            elif tag == 'circle':
                cx, cy, radius = (float(primitive.attrib[k]) for k in ('cx','cy','r'))
                commands = path_commands(f'M{cx+radius} {cy} A{radius} {radius} 0 1 1 {cx-radius} {cy} A{radius} {radius} 0 1 1 {cx+radius} {cy} Z')
            else:
                raise ValueError(tag)
            label = f'sv_icon_commands_{index}_{len(primitives)-first_primitive}'
            output.append('static const SvIconCommand '+label+'[] = {')
            for command in commands:
                op, values = command[0], list(command[1:])
                closed = int(values.pop()) if op == 'M' else 0
                values += [0]*(6-len(values))
                def number(v):
                    value = f'{v:.9g}'
                    return value+('.0' if '.' not in value and 'e' not in value else '')+'f'
                output.append("    {'"+op+"', "+str(closed)+', {'+', '.join(number(v) for v in values)+'}},')
            output.append('};')
            primitives.append((label, len(commands)))
        geometries.append((first_primitive,len(primitives)-first_primitive,float(item['stroke_width']),view[2],view[3]))
    output.append('static const SvIconPrimitive sv_icon_primitives[] = {')
    output.extend('    {'+name+', '+str(count)+'},' for name,count in primitives)
    output.append('};')
    output.append('static const SvIconGeometry sv_icon_geometry[SV_HUD_ICON_COUNT] = {')
    for first,count,stroke,w,h in geometries:
        output.append(f'    {{{first}, {count}, {stroke}f, {w}f, {h}f}},')
    output.extend(['};', '#define SV_HUD_ICON_SOURCE_GENERATION UINT64_C(0x'+generation+')', ''])
    (HERE.parent / 'hud-icons-data.h').write_text('\n'.join(output))
    manifest.pop('raster_size',None)
    manifest.pop('rasterizer',None)
    manifest['vector_format'] = SOURCE_FORMAT
    manifest['source_generation'] = generation
    (HERE / 'sources.json').write_text(json.dumps(manifest,indent=2)+'\n')
    print(f"Compiled {len(geometries)} original SVG geometries / {len(primitives)} stroke primitives")


compile_icons(snapshot())
