/* Prepared peer bytes; all decoding, delivery and drawing use production paths. */
#include "message-scenario.h"
#include "native-frame.h"
#include <stdio.h>
#include <string.h>
#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "Message scenario failed at line %d: %s\n", __LINE__, #test); \
    return 0; } } while (0)
int sv_message_scenario(SvApp *app, SvUi *ui)
{
    const unsigned char packet[] = {46,255,'w','E','c','h','o',0};
    const unsigned char hp[] = {24,0,100,0,75,0};
    unsigned cases = 0;
    for (int layout = 0; layout < 2; ++layout) {
        int version[6] = {4,7,0,2,0,layout + 1};
        for (size_t split = 0; split <= sizeof(packet); ++split) {
            unsigned char tail[32];
            SvMessage event;
            CHECK(sv_app_open(app, version) == SV_OK);
            uint64_t gen = sv_app_view(app).generation;
            CHECK(sv_app_receive(app, gen, packet, split) == SV_OK);
            CHECK(sv_app_step(app, 16).processed == (split == sizeof(packet) ? 1 : 0));
            if (split < sizeof(packet)) {
                CHECK(sv_app_take_message(app, gen, &event) == SV_WAITING);
                CHECK(!sv_app_view(app).messages.count);
            }
            size_t size = sizeof(packet) - split;
            memcpy(tail, packet + split, size);
            memcpy(tail + size, packet, sizeof(packet)); size += sizeof(packet);
            memcpy(tail + size, hp, layout ? 6 : 5); size += layout ? 6 : 5;
            CHECK(sv_app_receive(app, gen, tail, size) == SV_OK);
            sv_app_step(app, 16);
            SvAppView view = sv_app_view(app);
            CHECK(view.messages.count == 2 && view.status.current == 75);
            CHECK(view.messages.lines[0].sequence == 1 && view.messages.lines[1].sequence == 2);
            CHECK(sv_scenario_frame(ui, view));
            /* Recreate only the surface; undelivered effects belong to Session. */
            SvUi rebuilt = {.window = ui->window, .renderer = ui->renderer,
                            .font = ui->font, .font_revision = ui->font_revision};
            CHECK(sv_scenario_frame(&rebuilt, sv_app_view(app)));
            for (unsigned occurrence = 1; occurrence <= 2; ++occurrence) {
                CHECK(sv_app_take_message(app, gen, &event) == SV_OK);
                CHECK(event.sequence == occurrence && event.length == 6);
                CHECK(!memcmp(event.bytes, packet + 1, 7));
            }
            CHECK(sv_scenario_frame(&rebuilt, sv_app_view(app)));
            CHECK(sv_app_take_message(app, gen, &event) == SV_WAITING);
            CHECK(sv_app_view(app).status.revision == 1);
            ++cases;
        }
    }
    CHECK(sv_app_close(app) == SV_OK);
    printf("SV message checks passed cases=%u submitted_frames=%u\n", cases, cases * 3);
    return 1;
}
