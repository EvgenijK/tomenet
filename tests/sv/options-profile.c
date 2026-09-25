#include "options.h"
#include "resource.h"
#include "../../src/common/pack.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void value(const SvOptions *options, const char *name, bool expected)
{
    bool actual = !expected;
    assert(sv_options_get(options, name, &actual) && actual == expected);
}

static void packet(const SvOptions *options, const int version[6], size_t expected)
{
    unsigned char bytes[SV_OPTION_COUNT + 1] = {0};
    assert(sv_options_packet(options, version, bytes, expected - 1) == 0);
    assert(sv_options_packet(options, version, bytes, sizeof(bytes)) == expected);
    assert(bytes[0] == PKT_OPTIONS);
    assert(bytes[3] == 0); /* character X:censor_swearing, slot 2 */
    assert(bytes[47] == 1); /* one-to-many basic_players, slot 46 */
    assert(bytes[61] == 0); /* stack_allow_wands alias, slot 60 */
}

int main(int argc, char **argv)
{
    assert(argc == 3);
    SvOptions options;
    assert(sv_options_load_base(&options, argv[1]));
    value(&options, "censor_swearing", true);
    value(&options, "new_retaliator", true);
    value(&options, "basic_players_symb", true);
    value(&options, "basic_players_col", false);
    value(&options, "stack_allow_devices", false);
    value(&options, "subterm_flicker", false);
    value(&options, "autoloot_dunonly", false);
    value(&options, "add_kind_diz", false);
    value(&options, "auto_inscr_server", true);
    value(&options, "highlight_chat", false);
    value(&options, "highbeep_chat", false);
    value(&options, "view_animated_light", false);
    value(&options, "view_light_extra", false);
    value(&options, "no_light_fainting", true);
    value(&options, "highlight_player", true);
    value(&options, "colourize_bignum", true);
    value(&options, "sn_huge_bar", false);
    value(&options, "auto_inscr_off", true);
    assert(!sv_options_load_character(&options, argv[1], "../legacy"));
    assert(sv_options_load_character(&options, argv[1], "Hero"));
    value(&options, "censor_swearing", false);
    value(&options, "new_retaliator", true);
    const int old[6] = {4, 5, 5, 0, 0, 0};
    const int compat[6] = {4, 5, 5, 0, 0, 1};
    const int middle[6] = {4, 5, 8, 1, 0, 2};
    const int newest[6] = {4, 9, 1, 0, 0, 1};
    packet(&options, old, 97);
    packet(&options, compat, 129);
    packet(&options, middle, 155);
    packet(&options, newest, 200);

    SvResourceRef ref, target, temp;
    SDL_PathInfo info;
    assert(!sv_resource_path(argv[1], argv[2], "xtra/../tomenet.cfg",
                             SV_RESOURCE_USER, &ref));
    assert(sv_resource_find(argv[1], argv[2], "xtra/font/test.ttf",
                            SDL_PATHTYPE_FILE, &ref));
    assert(ref.owner == SV_RESOURCE_USER);
    assert(sv_resource_stat(&ref, &info) && info.type == SDL_PATHTYPE_FILE);
    assert(sv_resource_remove(&ref));
    assert(sv_resource_find(argv[1], argv[2], "xtra/font/test.ttf",
                            SDL_PATHTYPE_FILE, &ref));
    assert(ref.owner == SV_RESOURCE_BUNDLED);
    SvResourceRef bundled_temp;
    assert(!sv_resource_sidecar(&ref, ".tmp", &bundled_temp));
    assert(!sv_resource_remove(&ref));
    assert(sv_resource_stat(&ref, &info) && info.size == 7);
    assert(sv_resource_path(argv[1], argv[2], "xtra/font/renamed.ttf",
                            SV_RESOURCE_BUNDLED, &bundled_temp));
    assert(!sv_resource_rename(&ref, &bundled_temp));
    assert(sv_resource_stat(&ref, &info) && info.size == 7);
    assert(!sv_resource_stat(&bundled_temp, &info));
    assert(sv_resource_write_target(argv[1], "xtra/font/test.ttf", &target));
    assert(target.owner == SV_RESOURCE_USER);
    assert(sv_resource_sidecar(&target, ".tmp", &temp));
    assert(temp.owner == target.owner);
    FILE *stream = fopen(temp.path, "wb");
    assert(stream && fputs("replacement", stream) >= 0 && !fclose(stream));
    assert(sv_resource_rename(&temp, &target));
    assert(sv_resource_stat(&target, &info) && info.size == 11);
    assert(!sv_resource_rename(&target, &ref)); /* cannot cross owner */
    assert(sv_resource_remove(&target));
    assert(sv_resource_find(argv[1], argv[2], "xtra/font/test.ttf",
                            SDL_PATHTYPE_FILE, &ref) && ref.owner == SV_RESOURCE_BUNDLED);
    puts("SV-B-003 options/resource production seam checks passed");
    return 0;
}
