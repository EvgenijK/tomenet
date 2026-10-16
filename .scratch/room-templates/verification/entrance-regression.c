/* Harness support only. Actual parser, transforms, route and build are inserted
 * from current production source by entrance-regression.py. */
#define SERVER
#include "angband.h"
#include <assert.h>
#include <errno.h>
#include <stdarg.h>

#define BLOCK_HGT 11
#define BLOCK_WID 11
#define CENT_MAX 100
#define DOOR_MAX 400
#define WALL_MAX 1000
#define TUNN_MAX 1800
#define MAX_ROOMS_ROW (MAX_HGT / BLOCK_HGT)
#define MAX_ROOMS_COL (MAX_WID / BLOCK_WID)
#define DUN_TUN_CHG 30
#define DUN_TUN_RND 10
#define DUN_TUN_CON 15
#define DUN_TUN_PEN 25
#define COMFORT_PASSAGE_DEPTH 5
#define TRAPPED_DOOR_RATE 400
#define TRAPPED_DOOR_BASE 30
/* PRODUCTION_TYPES */
static u16b feat_wall_outer = FEAT_WALL_OUTER;
static dungeon_info_type dungeon_metadata[1];
dungeon_info_type *d_info = dungeon_metadata;
static dungeon_type dungeon_settings;
feature_type *f_info;
u16b max_f_idx = MAX_F_IDX;
static feature_type features[MAX_F_IDX];
static cave_type cells[MAX_HGT][MAX_WID], *rows[MAX_HGT];
static int resolve_blocked = -1, rng_calls, first_orientation;
static char last_error[256];

static int harness_fgets(FILE *fp, char *buf, size_t length, bool convert) {
    if (!fgets(buf, length, fp)) return 1;
    buf[strcspn(buf, "\r\n")] = 0;
    return 0;
}
static char *harness_string(const char *text) {
    char *result = malloc(strlen(text) + 1); strcpy(result, text); return result;
}
static int room_template_error(cptr path, int line, cptr reason) {
    snprintf(last_error, sizeof(last_error), "%s", reason); return 1;
}
#define my_fopen(path, mode) fopen(path, mode)
#define my_fclose(fp) fclose(fp)
#define my_fgets(fp, buf, length, convert) harness_fgets(fp, buf, length, convert)
#define string_make(text) harness_string(text)
#define suffix(text, ending) (strlen(text) >= strlen(ending) && !strcmp(text + strlen(text) - strlen(ending), ending))
#define room_template_init_monster_profiles(room) ((void)0)
#define room_template_init_item_profiles(room) ((void)0)
#define room_template_parse_monster_profile(path, line, buf, room) (1)
#define room_template_parse_item_profile(path, line, buf, room) (1)
#define room_template_validate_monster_profiles(path, room) (0)
#define room_template_validate_item_profiles(path, room) (0)
/* PRODUCTION_PARSER */

static int harness_rand(int n) { rng_calls++; return first_orientation && n == first_orientation + 1 ? 0 : n - 1; }
static int harness_resolve(int feat) { return feat == resolve_blocked ? FEAT_WALL_SOLID : feat; }
static void harness_set(int y, int x, int feat) { cells[y][x].feat = feat; }
#undef rand_int
#define rand_int(n) harness_rand(n)
#define getcave(wpos) (rows)
#define getdungeon(wpos) (&dungeon_settings)
#undef in_irondeepdive
#define in_irondeepdive(wpos) (FALSE)
#define rand_dir(y, x) ((void)0)
#define place_random_door(wpos, y, x) ((void)0)
#define getlevel(wpos) (0)
#define place_trap(wpos, y, x, kind) ((void)0)
#define cave_set_feat_resolve(wpos, y, x, feat) harness_resolve(feat)
#define cave_set_feat(wpos, y, x, feat) harness_set(y, x, feat)
#define place_floor(wpos, y, x) harness_set(y, x, FEAT_FLOOR)
#define room_template_place_item(wpos, y, x, profile) (0)
#define room_template_place_monster(wpos, y, x, profile, feat) (0)
#define place_monster_companions(wpos, y, x, idx, asleep, a, b) ((void)0)
/* PRODUCTION_GENERATOR */

static void reset(void) {
    memset(&harness_dungeon, 0, sizeof(harness_dungeon));
    memset(cells, 0, sizeof(cells));
    harness_dungeon.row_rooms = MAX_HGT / BLOCK_HGT;
    harness_dungeon.col_rooms = MAX_WID / BLOCK_WID;
    resolve_blocked = -1; rng_calls = 0; first_orientation = 0;
    memset(dungeon_metadata, 0, sizeof(dungeon_metadata));
    memset(&dungeon_settings, 0, sizeof(dungeon_settings));
    dungeon_settings.flags1 = DF1_NO_DOORS;
    for (int i = 0; i < 5; i++) dungeon_metadata[0].fill_type[i] = 97;
    for (int y = 0; y < MAX_HGT; y++) {
        rows[y] = cells[y];
        for (int x = 0; x < MAX_WID; x++) cells[y][x].feat = 97;
    }
}
static room_template_type room(void) {
    room_template_type result; memset(&result, 0, sizeof(result));
    result.wid = 5; result.hgt = 3;
    result.terrain_map = harness_string("#####" "#...#" "#####");
    result.monster_map = harness_string("_______________");
    result.item_map = harness_string("_______________");
    result.terrain_feature['#'] = FEAT_WALL_EXTRA;
    result.terrain_feature['.'] = FEAT_FLOOR;
    result.entrances = calloc(15, sizeof(*result.entrances));
    result.entrance_count = 2;
    result.entrances[0].x = 0; result.entrances[0].y = 1; result.entrances[0].feat = FEAT_FLOOR;
    result.entrances[1].x = 4; result.entrances[1].y = 1; result.entrances[1].feat = FEAT_OPEN;
    return result;
}
static void discard(room_template_type *r) {
    free(r->name); free(r->terrain_map); free(r->monster_map); free(r->item_map); free(r->entrances);
}
static int parser_case(const char *directory, const char *entrances, int terrain, bool success, int count, int feat) {
    char path[1024]; snprintf(path, sizeof(path), "%s/example.txt", directory);
    FILE *fp = fopen(path, "w"); assert(fp);
    fprintf(fp, "V:1\nN:example:Example\nS:3:1\n%sT:#:%d\nT:.:%d\nDT:\n#.#\nDM:\n___\nDI:\n___\n", entrances, terrain, FEAT_FLOOR);
    fclose(fp);
    room_template_type r; int line; last_error[0] = 0;
    int status = room_template_parse_file(path, "example.txt", &r, &line);
    if (success != (status == 0)) fprintf(stderr, "parser unexpected %d: %s; input %s", status, last_error, entrances);
    assert(success == (status == 0));
    if (success) { assert(r.entrance_count == count); if (count) assert(r.entrances[0].feat == feat); }
    discard(&r); return 1;
}
static void parser_tests(const char *directory) {
    char text[256]; int cases = 0;
    snprintf(text, sizeof(text), "E:0:0:%d\n", FEAT_FLOOR);
    cases += parser_case(directory, text, FEAT_WALL_EXTRA, TRUE, 1, FEAT_FLOOR);
    snprintf(text, sizeof(text), "E:0:0:%d\nE:0:0:%d\n", FEAT_FLOOR, FEAT_FLOOR);
    cases += parser_case(directory, text, FEAT_WALL_EXTRA, TRUE, 1, FEAT_FLOOR);
    snprintf(text, sizeof(text), "E:0:0:%d\nE:0:0:%d\n", FEAT_FLOOR, FEAT_OPEN);
    cases += parser_case(directory, text, FEAT_WALL_EXTRA, FALSE, 0, 0);
    snprintf(text, sizeof(text), "E:1:0:%d\nE:1:0\n", FEAT_FLOOR);
    cases += parser_case(directory, text, FEAT_WALL_EXTRA, TRUE, 1, FEAT_FLOOR);
    snprintf(text, sizeof(text), "E:1:0\nE:1:0:%d\n", FEAT_FLOOR);
    cases += parser_case(directory, text, FEAT_WALL_EXTRA, TRUE, 1, FEAT_FLOOR);
    snprintf(text, sizeof(text), "E:0:0:%d\nE:0:0\n", FEAT_FLOOR);
    cases += parser_case(directory, text, FEAT_WALL_EXTRA, FALSE, 0, 0);
    snprintf(text, sizeof(text), "E:0:0\nE:0:0:%d\n", FEAT_FLOOR);
    cases += parser_case(directory, text, FEAT_WALL_EXTRA, FALSE, 0, 0);
    snprintf(text, sizeof(text), "E:0:0:%d\n", FEAT_DOOR_HEAD);
    cases += parser_case(directory, text, FEAT_WALL_EXTRA, TRUE, 1, FEAT_DOOR_HEAD);
    snprintf(text, sizeof(text), "E:0:0:%d\n", FEAT_WALL_EXTRA);
    cases += parser_case(directory, text, FEAT_DEEP_LAVA, TRUE, 1, FEAT_WALL_EXTRA);
    const int forbidden[] = { FEAT_WALL_SOLID, FEAT_LESS, MAX_F_IDX - 1};
    for (size_t i = 0; i < sizeof(forbidden)/sizeof(*forbidden); i++) {
        snprintf(text, sizeof(text), "E:0:0:%d\n", forbidden[i]);
        cases += parser_case(directory, text, FEAT_WALL_EXTRA, FALSE, 0, 0);
    }
    snprintf(text, sizeof(text), "E:0:0:%d\n", FEAT_FLOOR);
    cases += parser_case(directory, text, FEAT_DEEP_LAVA, TRUE, 1, FEAT_FLOOR);
    cases += parser_case(directory, "E:1:0\n", FEAT_WALL_EXTRA, TRUE, 1, FEAT_FLOOR);
    cases += parser_case(directory, "E:0:0\n", FEAT_WALL_EXTRA, FALSE, 0, 0);
    cases += parser_case(directory, "", FEAT_WALL_EXTRA, TRUE, 0, 0);
    const char *invalid[] = {"E:0:0:-1\n", "E:0:0:999999\n", "E:0:0:abc\n", "E:0:0:\n", "E:3:0:1\n", "E:0:0:1:2\n"};
    for (size_t i = 0; i < sizeof(invalid)/sizeof(*invalid); i++) cases += parser_case(directory, invalid[i], FEAT_WALL_EXTRA, FALSE, 0, 0);
    printf("PARSER_PASS %d cases (explicit wall, legacy, duplicates, conflicts, malformed)\n", cases);
}
static void entry_position(room_template_placement *placement, int index, int *y, int *x) {
    room_template_entrance *entry = &placement->room->entrances[index];
    room_template_transform(placement->room, placement->orientation, entry->x, entry->y, x, y);
    *y += placement->top; *x += placement->left;
}
static void unchanged_entries(room_template_placement *placement) {
    int y, x;
    for (int i = 0; i < placement->room->entrance_count; i++) {
        entry_position(placement, i, &y, &x);
        assert(cells[y][x].feat == FEAT_WALL_EXTRA);
    }
}
static void placement_tests(void) {
    room_template_type r = room(); worldpos pos = {0};
    for (int orientation = 0; orientation < 8; orientation++) {
        reset(); first_orientation = orientation;
        for (int y = 0; y < MAX_HGT; y++) for (int x = 0; x < MAX_WID; x++) cells[y][x].info = CAVE_NO_TFORM;
        assert(build_room_template(&pos, 2, 3, &r));
        assert(dun->cent_n == 1 && dun->template_n == 1);
        assert(dun->templates[0].used_entrance == -1);
        unchanged_entries(&dun->templates[0]);
    }
    reset(); r.entrance_count = 0;
    assert(build_room_template(&pos, 2, 3, &r));
    assert(dun->cent_n == 0 && dun->template_n == 0);
    assert(cells[27][36].feat == FEAT_WALL_EXTRA && cells[27][40].feat == FEAT_WALL_EXTRA);
    reset(); r.entrance_count = 2; memset(r.terrain_map, '#', 15);
    resolve_blocked = FEAT_FLOOR;
    assert(build_room_template(&pos, 2, 3, &r));
    unchanged_entries(&dun->templates[0]);
    discard(&r);
    puts("PLACEMENT_PASS no existing centers, impassable surrounds, no entrance replacement, all 8 transforms, zero optional entrances, unresolvable replacement does not block placement");
}
static void tunnel_tests(void) {
    worldpos pos = {0}; room_template_type r = room();
    reset(); cells[10][10].feat = FEAT_FLOOR; cells[10][10].info = CAVE_ICKY;
    assert(tunnel_cell_is_connectable(&pos, &cells[10][10]));
    cells[10][10].feat = 97;
    assert(!tunnel_cell_is_connectable(&pos, &cells[10][10]));
    const int replacements[] = {FEAT_DOOR_HEAD, FEAT_WALL_INNER, FEAT_DEEP_LAVA, FEAT_FLOOR};
    for (int orientation = 0; orientation < 8; orientation++) {
        reset(); first_orientation = orientation;
        r.entrances[0].feat = replacements[orientation % 4];
        assert(build_room_template(&pos, 2, 3, &r));
        room_template_placement *placement = &dun->templates[0];
        unchanged_entries(placement);
        int y, x, width, height; entry_position(placement, 0, &y, &x);
        room_template_dimensions(&r, orientation, &width, &height);
        coord target = {y, x};
        if (x == placement->left) target.x -= 6;
        else if (x == placement->left + width - 1) target.x += 6;
        else if (y == placement->top) target.y -= 6;
        else target.y += 6;
        cells[target.y][target.x].feat = FEAT_FLOOR;
        cells[target.y][target.x].info = CAVE_ROOM;
        if (orientation & 1) room_template_connect_rooms(&pos, target, placement->center);
        else room_template_connect_rooms(&pos, placement->center, target);
        assert(placement->used_entrance == 0);
        assert(cells[y][x].feat == replacements[orientation % 4]);
        assert(cells[y][x].info2 & CAVE2_ROOM_TEMPLATE);
        entry_position(placement, 1, &y, &x);
        assert(cells[y][x].feat == FEAT_WALL_EXTRA);
        assert(dun->tunn_n > 0); /* Actual build_tunnel carved PERMANENT dungeon fill97. */
    }
    reset(); r.entrances[0].feat = FEAT_FLOOR;
    assert(build_room_template(&pos, 2, 3, &r));
    room_template_placement *placement = &dun->templates[0];
    coord target = {27, 50}; cells[27][50].feat = FEAT_FLOOR;
    resolve_blocked = FEAT_FLOOR;
    room_template_connect_rooms(&pos, placement->center, target);
    assert(placement->used_entrance == 1);
    int y, x; entry_position(placement, 0, &y, &x); assert(cells[y][x].feat == FEAT_WALL_EXTRA);
    entry_position(placement, 1, &y, &x); assert(cells[y][x].feat == FEAT_OPEN);
    reset(); assert(build_room_template(&pos, 2, 3, &r));
    placement = &dun->templates[0]; target.x = 5;
    for (int yy = 0; yy < MAX_HGT; yy++) cells[yy][20].feat = FEAT_WALL_SOLID;
    room_template_connect_rooms(&pos, placement->center, target);
    assert(placement->used_entrance == -1); unchanged_entries(placement);
    /* A room-marked arbitrary wall must not be treated as traversable. */
    reset(); assert(build_room_template(&pos, 2, 3, &r)); placement = &dun->templates[0];
    for (int yy = 0; yy < MAX_HGT; yy++) {
        cells[yy][20].feat = FEAT_WALL_INNER;
        cells[yy][20].info = CAVE_ROOM;
    }
    room_template_connect_rooms(&pos, placement->center, target);
    assert(placement->used_entrance == -1); unchanged_entries(placement);
    /* Permanent non-fill cannot be accepted as a template exterior endpoint. */
    reset(); assert(build_room_template(&pos, 2, 3, &r)); placement = &dun->templates[0];
    for (int yy = 0; yy < MAX_HGT; yy++) for (int xx = 0; xx < MAX_WID; xx++)
        if (!(cells[yy][xx].info2 & CAVE2_ROOM_TEMPLATE)) cells[yy][xx].feat = FEAT_PERM_INNER;
    coord endpoint;
    assert(!room_template_tunnel_endpoint(&pos, rows, placement, 0, target, &endpoint));
    room_template_connect_rooms(&pos, placement->center, target);
    assert(placement->used_entrance == -1); unchanged_entries(placement);
    /* Two templates connect through the same ordinary tunnel phase. */
    reset(); assert(build_room_template(&pos, 2, 3, &r));
    assert(build_room_template(&pos, 2, 5, &r));
    room_template_connect_rooms(&pos, dun->templates[0].center, dun->templates[1].center);
    assert(dun->templates[0].used_entrance == 1 && dun->templates[1].used_entrance == 0);
    for (int i = 0; i < 2; i++) {
        placement = &dun->templates[i];
        entry_position(placement, placement->used_entrance, &y, &x);
        assert(cells[y][x].feat == r.entrances[placement->used_entrance].feat);
        entry_position(placement, 1 - placement->used_entrance, &y, &x);
        assert(cells[y][x].feat == FEAT_WALL_EXTRA);
    }
    /* WIDE_CORRIDORS must not queue or overwrite a protected side cell. */
    reset(); cells[28][25].feat = FEAT_WALL_EXTRA;
    cells[28][25].info2 = CAVE2_ROOM_TEMPLATE;
    cells[27][25].info = CAVE_ROOM; /* Fill marked as room still must be carved. */
    assert(build_tunnel(&pos, 27, 20, 27, 30, TRUE));
    assert(cells[27][25].feat == FEAT_FLOOR);
    assert(cells[28][25].feat == FEAT_WALL_EXTRA);
    reset(); cells[27][25].feat = FEAT_WALL_OUTER; cells[27][25].info = CAVE_ROOM;
    cells[28][25].feat = FEAT_WALL_OUTER; cells[28][25].info2 = CAVE2_ROOM_TEMPLATE;
    assert(build_tunnel(&pos, 27, 20, 27, 30, TRUE));
    assert(cells[27][25].feat == FEAT_FLOOR);
    assert(cells[28][25].feat == FEAT_WALL_OUTER);
    discard(&r);
    puts("TUNNEL_PASS actual build_tunnel over permanent fill97, all 8 transforms, exact wall/lava/door/floor, unused unchanged, resolution fallback, failed tunnel keeps entrances, arbitrary room wall/nonfill blocked, template-to-template, wide sidewall protection");
}
int main(int argc, char **argv) {
    assert(argc == 2); f_info = features;
    for (int i = 0; i < MAX_F_IDX; i++) features[i].name = 1;
    features[FEAT_FLOOR].flags1 = FF1_FLOOR;
    features[FEAT_OPEN].flags1 = FF1_FLOOR;
    features[FEAT_DOOR_HEAD].flags1 = FF1_DOOR;
    features[MAX_F_IDX - 1].name = 0;
    features[FEAT_WALL_SOLID].flags1 = FF1_PERMANENT;
    features[97].flags1 = FF1_PERMANENT;
    features[FEAT_PERM_INNER].flags1 = FF1_PERMANENT;
    parser_tests(argv[1]); placement_tests(); tunnel_tests();
    puts("ENTRANCE_REGRESSION_PASS");
}
