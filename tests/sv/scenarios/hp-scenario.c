/* Prepared external peer fixtures with independently specified semantic outcomes. */
#include "hp-scenario.h"
#include "ui/status.h"
#include <stdio.h>
#include <string.h>

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "HP scenario failed at line %d: %s\n", __LINE__, #test); \
    goto failed; } } while (0)

/* Feed and advance through the same application interfaces as the shell. */
static int feed(SvApp *app, const void *bytes, size_t size)
{
    SvResult result = sv_app_receive(app, sv_app_view(app).generation, bytes, size);
    if (result != SV_OK) return -1;
    SvStep step = sv_app_step(app, 16);
    return step.result == SV_OK || step.result == SV_WAITING || step.result == SV_RECOVERED ? 0 : -1;
}
int sv_hp_scenario(SvApp *s, SvScenarioFrame submit, void *context)
{
    static const struct {
        unsigned char bytes[6];
        int maximum, current, bar, boosted, drain;
        const char *old_text, *new_text;
    } cases[] = {
        {{24, 0, 100, 0, 75, 0}, 100, 75, 0, 0, 0, "HP 75 / 100", "HP 75 / 100"},
        {{24, 0x27, 0x74, 0x27, 0x42, 1}, 100, 50, 1, 1, 1, "HP 50 / 100  boosted  bar", "HP 50 / 100  boosted  bar  drain"},
        {{24, 0, 100, 0x27, 0x06, 255}, 100, -10, 0, 1, 255, "HP -10 / 100  boosted", "HP -10 / 100  boosted  drain"},
        {{24, 0x27, 0x10, 0x13, 0x88, 0}, 10000, 5000, 0, 0, 0, "HP 5000 / 10000", "HP 5000 / 10000"},
        {{24, 0x27, 0x11, 0x13, 0x89, 0}, 1, -4999, 1, 1, 0, "HP -4999 / 1  boosted  bar", "HP -4999 / 1  boosted  bar"},
        {{24, 0xff, 0xff, 0x80, 0, 0}, -1, -32768, 0, 0, 0, "HP -32768 / -1", "HP -32768 / -1"}
    };
    static const unsigned char sentinel[] = {24, 0, 90, 0, 30, 0};
    unsigned count = 0;
    for (int layout = 0; layout < 2; ++layout) {
        int version[6] = {4, 7, 0, 2, 0, layout ? 2 : 1};
        size_t length = layout ? 6 : 5;
        for (unsigned c = 0; c < sizeof(cases) / sizeof(cases[0]); ++c) {
            for (size_t split = 0; split <= length; ++split) {
                unsigned char tail[7], output[32];
                SvStatus view;
                char visible[160];
                CHECK(sv_app_open(s, version) == SV_OK);
                CHECK(s);
                CHECK(feed(s, cases[c].bytes, split) == 0);
                view = sv_app_view(s).status;
                if (split < length) {
                    CHECK(!view.available && view.revision == 0 && view.current == 0 && view.maximum == 0);
                    CHECK(feed(s, NULL, 0) == 0);
                    CHECK(sv_app_view(s).status.revision == 0);
                }
                /* Completion shares the receive buffer with a following known packet. */
                memcpy(tail, cases[c].bytes + split, length - split);
                tail[length - split] = sentinel[0];
                CHECK(feed(s, tail, length - split + 1) == 0);
                view = sv_app_view(s).status;
                CHECK(view.available && view.revision == 1);
                CHECK(view.maximum == cases[c].maximum && view.current == cases[c].current);
                CHECK(view.bar == cases[c].bar && view.boosted == cases[c].boosted);
                CHECK(view.drain == (layout ? cases[c].drain : 0));
                CHECK(sv_app_take_output(s, sv_app_view(s).generation, output, sizeof(output)).size == 0);
                sv_status_text(view, visible, sizeof(visible));
                CHECK(!strcmp(visible, layout ? cases[c].new_text : cases[c].old_text));
                CHECK(submit(context, sv_app_view(s)));
                CHECK(feed(s, sentinel + 1, length - 1) == 0);
                view = sv_app_view(s).status;
                CHECK(view.revision == 2 && view.maximum == 90 && view.current == 30);
                CHECK(!view.bar && !view.boosted && !view.drain);
                CHECK(feed(s, NULL, 0) == 0 && sv_app_view(s).status.revision == 2);
                CHECK(submit(context, sv_app_view(s)));
                CHECK(sv_app_close(s) == SV_OK);
                ++count;
            }
        }
    }
    {
        const int version[6] = {4, 7, 0, 2, 0, 2};
        const unsigned char unknown[] = {7, 24, 0, 100, 0, 75, 0};
        const unsigned char expected[] = {211, 0, 0, 0, 7, 0, 0, 0, 24, 111, 0};
        unsigned char output[32], overflow[1025] = {0};
        CHECK(sv_app_open(s, version) == SV_OK);
        CHECK(s && feed(s, sentinel, sizeof(sentinel)) == 0);
        CHECK(feed(s, unknown, sizeof(unknown)) == 0);
        CHECK(sv_app_view(s).status.revision == 1);
        CHECK(sv_app_take_output(s, sv_app_view(s).generation, output, sizeof(output)).size == sizeof(expected));
        CHECK(!memcmp(output, expected, sizeof(expected)));
        CHECK(feed(s, sentinel, sizeof(sentinel)) == 0);
        CHECK(sv_app_view(s).status.revision == 2);
        CHECK(feed(s, overflow, sizeof(overflow)) == -1);
        CHECK(feed(s, sentinel, sizeof(sentinel)) == -1);
        CHECK(sv_app_view(s).status.revision == 2);
        CHECK(sv_app_close(s) == SV_OK);
    }
    printf("SV HP checks passed cases=%u submitted_frames=%u layouts=4.7.0.2.0.1,4.7.0.2.0.2 fallback_routes=0\n", count, count * 2);
    return 1;
failed:
    sv_app_close(s);
    return 0;
}

/* Read back only during checks, before the production frame is submitted. */
bool sv_hp_check_pixels(SDL_Renderer *renderer, const SDL_Rect *bounds)
{
    SDL_Surface *pixels = SDL_RenderReadPixels(renderer, bounds);
    bool ink = false;
    if (!pixels) return false;
    for (int y = 0; y < pixels->h && !ink; ++y) {
        for (int x = 0; x < pixels->w; ++x) {
            Uint8 r, g, b, a;
            if (!SDL_ReadSurfacePixel(pixels, x, y, &r, &g, &b, &a)) {
                SDL_DestroySurface(pixels);
                return false;
            }
            if (r > 100 && g > 100 && b > 100) { ink = true; break; }
        }
    }
    SDL_DestroySurface(pixels);
    return ink || SDL_SetError("HP surface submitted without visible text pixels");
}
