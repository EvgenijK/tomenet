#!/usr/bin/env python3
"""Compile current production parser and placement functions in a throwaway harness.

Uses real server types/constants; stubs I/O, feature setters and unrelated content
placement. Never loads or modifies a running server, saved floor or live account.
"""
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
generator = (ROOT / 'src/server/generate.c').read_text()
parser = (ROOT / 'src/server/init2.c').read_text()

def function(source, name):
    start = re.search(r'^(?:static )?(?:bool|void|int|errr|room_template_placement \*)[ \t]*' + name + r'\(', source, re.M).start()
    opening = source.index('{', start)
    level, end = 1, opening + 1
    while level:
        level += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

orientation = generator[generator.index('typedef enum {\n\tROOM_TEMPLATE_ORIENTATION_0'):generator.index('static void room_template_dimensions')]
parse_state_start = parser.index('typedef enum {\n\tROOM_TEMPLATE_PARSE_HEADER')
parse_state = parser[parse_state_start:parser.index('static errr room_template_parse_file', parse_state_start)]
source = (Path(__file__).with_suffix('.c')).read_text()
coord_start = generator.index('typedef struct coord coord;')
coord_end = generator.index('};', coord_start) + 2
placement_start = generator.index('typedef struct {\n\troom_template_type *room;')
placement_end = generator.index('} room_template_placement;', placement_start) + len('} room_template_placement;')
dun_start = generator.index('typedef struct dun_data dun_data;')
dun_end = generator.index('};', dun_start) + 2
source = source.replace('/* PRODUCTION_TYPES */', generator[coord_start:coord_end] + '\n' + generator[placement_start:placement_end] + '\n' + generator[dun_start:dun_end] + '\nstatic dun_data harness_dungeon, *dun = &harness_dungeon;')
source = source.replace('/* PRODUCTION_PARSER */', parse_state + '\n'.join(function(parser, name) for name in [
    'room_template_number', 'room_template_symbol', 'room_template_terrain_allowed',
    'room_template_terrain_passable', 'room_template_parse_file']))
source = source.replace('/* PRODUCTION_GENERATOR */', orientation + '\n'.join(function(generator, name) for name in [
    'tunnel_is_dungeon_fill', 'tunnel_cell_is_connectable',
    'room_template_dimensions', 'room_template_transform', 'room_template_connection_point',
    'room_template_blocks', 'build_room_template', 'correct_dir', 'build_tunnel',
    'room_template_at_center', 'room_template_tunnel_endpoint', 'room_template_use_entrance',
    'room_template_connect_rooms']))
with tempfile.TemporaryDirectory(prefix='entrance-regression-') as directory:
    directory = Path(directory)
    (directory / 'harness.c').write_text(source)
    subprocess.run(['cc', '-std=c99', '-D_DEFAULT_SOURCE', '-DMEXP=19937', '-DENABLE_ROOM_TEMPLATES',
                    '-I' + str(ROOT / 'src/server'), '-I' + str(ROOT / 'src/server/lua'),
                    '-O1', '-g', '-Wall', '-Wextra', '-Wno-unused-parameter', '-Wno-unused-variable',
                    '-Wno-stringop-truncation', '-Werror', '-fsanitize=undefined', '-fno-sanitize-recover=all',
                    str(directory / 'harness.c'), '-o', str(directory / 'harness')], check=True)
    subprocess.run([str(directory / 'harness'), str(directory)], check=True)
