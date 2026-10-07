#include "ui/endpoint-scene.h"
#include "session/login-view.h"
#include "session/birth.h"
#include <stdio.h>
#include <string.h>

static bool line(SDL_Renderer *renderer, SvFont *font, const char *text,
                 int x, int y, float scale, SDL_Color color)
{
    char printable[256];
    size_t i = 0;
    for (; text[i] && i + 1 < sizeof(printable); ++i) {
        unsigned char c = (unsigned char)text[i];
        printable[i] = c >= 32 && c <= 126 ? (char)c : '?';
    }
    printable[i] = 0;
    return sv_font_draw(font, renderer, printable, (int)(x * scale), (int)(y * scale), scale, color);
}

bool sv_endpoint_render(SDL_Renderer *renderer, SvFont *font,
                        const SvEndpoint *endpoint, const SvEndpointInput *input)
{
    int width, height;
    if (!SDL_GetCurrentRenderOutputSize(renderer, &width, &height)) return false;
    float scale = SDL_GetWindowDisplayScale(SDL_GetRenderWindow(renderer));
    if (scale <= 0) scale = 1.0f;
    const SDL_Color title = {226,235,245,255}, normal = {185,205,223,255},
                    selected = {255,210,106,255}, disabled = {92,108,121,255},
                    error = {255,135,135,255};
    if (!SDL_SetRenderDrawColor(renderer, 15,21,28,255) || !SDL_RenderClear(renderer)) return false;
    if (!line(renderer,font,"TomeNET SV - Server",40,34,scale,title)) return false;
    if (input->temporary_surface_text)
        return line(renderer, font, input->temporary_surface_text,
                    40, 180, scale, normal);
    if (endpoint->phase == SV_ENDPOINT_LIST) {
        if (!line(renderer,font,"Select a server, or press Q for manual address. Escape exits.",40,85,scale,normal)) return false;
        if (!endpoint->server_count &&
            !line(renderer,font,"No server list available. Press Q to enter a host.",40,130,scale,error)) return false;
        int available = (int)(height / scale - 140) / 28;
        size_t rows = available > 0 ? (size_t)available : 1;
        if (rows > 20) rows = 20;
        size_t first = endpoint->selected < endpoint->server_count && endpoint->selected >= rows ?
            endpoint->selected - rows + 1 : 0;
        for (size_t i = first; i < endpoint->server_count && i < first + rows; ++i) {
            char row[240];
            char ping[32];
            const SvServer *server = &endpoint->servers[i];
            if (server->ping_ms == -1) strcpy(ping, "ping pending");
            else if (server->ping_ms < 0) strcpy(ping, "ping unavailable");
            else snprintf(ping,sizeof(ping),"%d ms",server->ping_ms);
            snprintf(row,sizeof(row),"%zu) %.79s : %u  %.80s  %s",i + 1,server->host,
                     (unsigned)server->port,server->label,ping);
            if (!line(renderer,font,row,56,140 + (int)(i - first)*28,scale,
                      i == endpoint->selected ? selected : normal)) return false;
        }
    } else if (endpoint->phase == SV_ENDPOINT_MANUAL) {
        const char *label = "Host [:port]: ";
        if (!line(renderer,font,"Enter server hostname or IP address. Escape exits.",40,85,scale,normal) ||
            !line(renderer,font,label,40,148,scale,normal)) return false;
        int cell = sv_font_cell_width(font, scale);
        if (cell <= 0) return false;
        float field_x = 40 * scale + strlen(label) * (size_t)cell;
        size_t visible = (size_t)((width - field_x - 40 * scale) / cell);
        if (!visible) return false;
        size_t start = endpoint->editor.cursor >= visible ? endpoint->editor.cursor - visible + 1 : 0;
        size_t count = endpoint->editor.length - start;
        if (count > visible) count = visible;
        char draft[SV_HOST_LIMIT + 1];
        memcpy(draft,endpoint->editor.bytes + start,count);
        draft[count] = 0;
        size_t selected_start = endpoint->editor.cursor < endpoint->editor.anchor ?
            endpoint->editor.cursor : endpoint->editor.anchor;
        size_t selected_end = endpoint->editor.cursor > endpoint->editor.anchor ?
            endpoint->editor.cursor : endpoint->editor.anchor;
        if (selected_start < start) selected_start = start;
        if (selected_end > start + count) selected_end = start + count;
        if (selected_start < selected_end) {
            SDL_FRect selection = {field_x + (selected_start - start) * (size_t)cell,
                                   146 * scale, (selected_end - selected_start) * (size_t)cell, 26 * scale};
            if (!SDL_SetRenderDrawColor(renderer,57,79,104,255) ||
                !SDL_RenderFillRect(renderer,&selection)) return false;
        }
        if (count && !line(renderer,font,draft,(int)(field_x / scale),148,scale,title)) return false;
        SDL_FRect caret = {field_x + (endpoint->editor.cursor - start) * (size_t)cell,
                          172.0f * scale, (float)cell, 2.0f * scale};
        if (!SDL_SetRenderDrawColor(renderer,255,210,106,255) || !SDL_RenderFillRect(renderer,&caret)) return false;
        if (endpoint->invalid_address &&
            !line(renderer,font,"Invalid host or port. Edit the draft and try again.",40,212,scale,error)) return false;
        if (input->text_error == SV_TEXT_ENCODING_ERROR &&
            !line(renderer,font,"Cannot encode that character as a server field byte.",40,245,scale,error)) return false;
        if (input->clipboard_unavailable &&
            !line(renderer,font,"Clipboard unavailable; address unchanged.",40,278,scale,error)) return false;
        if (endpoint->editor.searching) {
            const char *match = endpoint->editor.preview[0] ? endpoint->editor.preview : "No history match";
            if (!line(renderer,font,match,40,318,scale,normal)) return false;
        }
    } else if (endpoint->phase == SV_ENDPOINT_SELECTED) {
        char row[160];
        snprintf(row,sizeof(row),"Selected: %.79s : %u",endpoint->host,(unsigned)endpoint->port);
        if (!line(renderer,font,row,40,120,scale,selected) ||
            !line(renderer,font,input->contact_status ? input->contact_status :
                  "Account credentials are required for contact.",40,164,scale,normal)) return false;
        if (input->text_error == SV_TEXT_ENCODING_ERROR &&
            !line(renderer,font,"Cannot encode that character as a server field byte.",40,203,scale,error)) return false;
        if (input->clipboard_unavailable &&
            !line(renderer,font,"Clipboard unavailable; credential draft unchanged.",40,240,scale,error)) return false;
        if (input->login_view && input->login_view->overview) {
            for (size_t i = 0; i < input->login_view->count; ++i) {
                const SvLoginRowView *character = &input->login_view->rows[i];
                char row[256];
                snprintf(row, sizeof(row), "%c) %.79s, level %d, %.40s %.40s, mode %d  %.50s",
                         (int)('a' + i), character->name, character->level,
                         character->race, character->class_title, character->mode,
                         character->location);
                if (!line(renderer,font,row,40,275 + (int)i * 25,scale,normal)) return false;
            }
            if (input->login_view->ordinary_name_entry) {
                char prompt[128];
                snprintf(prompt, sizeof(prompt), "New character name: %.79s",
                         input->login_view->character_name_draft ?
                         input->login_view->character_name_draft : "");
                if (!line(renderer,font,prompt,40,690,scale,selected) ||
                    !line(renderer,font,"Escape cancels",40,720,scale,normal)) return false;
            }
        }
        if (input->login_view && input->login_view->motd && input->login_view->motd_size) {
            size_t at = 0;
            for (int row = 0; row < 23 && at < input->login_view->motd_size; ++row) {
                char text[121];
                size_t count = input->login_view->motd_size - at;
                if (count > 120) count = 120;
                memcpy(text, input->login_view->motd + at, count);
                text[count] = 0;
                if (!line(renderer,font,text,40,270 + row * 20,scale,normal)) return false;
                at += count;
            }
        }
        if (input->login_view && input->login_view->birth) {
            if (!line(renderer,font,"Character name accepted",40,240,scale,selected))
                return false;
            if (input->birth_view && input->birth_view->owner == SV_BIRTH_SEX) {
                if (!line(renderer,font,"Choose sex (* random, # DNA, Q quits)",40,270,
                          scale,normal) ||
                    !line(renderer,font,"Male",60,310,scale,
                          input->birth_view->highlighted_sex == SV_BIRTH_SEX_MALE ?
                          selected : normal) ||
                    !line(renderer,font,"Female",60,360,scale,
                          input->birth_view->highlighted_sex == SV_BIRTH_SEX_FEMALE ?
                          selected : normal)) return false;
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_CLASS) {
                if (!line(renderer,font,"Choose class (* random, # DNA, Q quits)",40,290,
                          scale,normal)) return false;
                for (unsigned i = 0; i < input->birth_view->legal_class_count; ++i) {
                    unsigned id = input->birth_view->legal_classes[i];
                    if (!line(renderer,font,input->birth_view->setup->classes[id].title,
                              60,340 + (int)i * 40,scale,
                              input->birth_view->highlighted_class == (int)id ?
                              selected : normal)) return false;
                }
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_RACE) {
                if (!line(renderer,font,"Choose race (* random, # DNA, Q quits)",40,290,
                          scale,normal)) return false;
                for (unsigned id = 0; id < input->birth_view->setup->race_count; ++id) {
                    bool legal = false;
                    for (unsigned i = 0; i < input->birth_view->legal_race_count; ++i)
                        if (input->birth_view->legal_races[i] == id) legal = true;
                    if (!line(renderer,font,input->birth_view->setup->races[id].title,
                              60,340 + (int)id * 40,scale,
                              input->birth_view->highlighted_race == (int)id ?
                              selected : legal ? normal : disabled)) return false;
                }
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_TRAIT) {
                if (!line(renderer,font,"Choose trait (* random, # DNA, Q quits)",40,290,
                          scale,normal)) return false;
                for (unsigned i = 0; i < input->birth_view->legal_trait_count; ++i) {
                    unsigned id = input->birth_view->legal_traits[i];
                    if (!line(renderer,font,input->birth_view->setup->traits[id].title,
                              60,340 + (int)i * 40,scale,
                              input->birth_view->highlighted_trait == (int)id ?
                              selected : normal)) return false;
                }
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_BODY) {
                if (!line(renderer,font,"Choose body (* random, # DNA, Q quits)",40,290,
                          scale,normal)) return false;
                for (unsigned i = 0; i < input->birth_view->legal_body_count; ++i) {
                    SvBirthBody body = input->birth_view->legal_bodies[i];
                    const char *title = body == SV_BIRTH_BODY_FRUIT_BAT ?
                        "Fruit bat" : "Normal body";
                    if (!line(renderer,font,title,60,340 + (int)i * 40,scale,
                              input->birth_view->highlighted_body == body ?
                              selected : normal)) return false;
                }
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_STATS) {
                static const char *names[] = {
                    "Strength", "Intelligence", "Wisdom", "Dexterity",
                    "Constitution", "Charisma"
                };
                char prompt[96];
                if (input->birth_view->stats_confirmation_pending)
                    snprintf(prompt, sizeof(prompt),
                             "%d points remain; confirm again to continue",
                             input->birth_view->stats_remaining);
                else if (input->birth_view->stats_point_buy)
                    snprintf(prompt, sizeof(prompt),
                             "Distribute stats (%d points left, # DNA, Q quits)",
                             input->birth_view->stats_remaining);
                else snprintf(prompt, sizeof(prompt),
                              "Choose stat order (* random, Q quits)");
                if (!line(renderer,font,prompt,40,290,scale,normal)) return false;
                for (unsigned i = 0; i < input->birth_view->legal_stat_count; ++i) {
                    unsigned stat = input->birth_view->legal_stats[i];
                    char row[96];
                    if (input->birth_view->stats_point_buy)
                        snprintf(row, sizeof(row), "%s: %d", names[stat],
                                 input->birth_view->stats[stat]);
                    else snprintf(row, sizeof(row), "%s", names[stat]);
                    if (!line(renderer,font,row,60,340 + (int)i * 40,scale,
                              input->birth_view->highlighted_stat == (int)stat ?
                              selected : normal)) return false;
                }
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_MODE) {
                if (!line(renderer,font,"Choose mode (* random, # DNA, Q quits)",40,290,
                          scale,normal)) return false;
                for (unsigned i = 0; i < input->birth_view->legal_mode_count; ++i) {
                    SvBirthMode mode = input->birth_view->legal_modes[i];
                    const char *title = mode == SV_BIRTH_MODE_NORMAL ? "Normal" :
                        mode == SV_BIRTH_MODE_NO_GHOST ? "No Ghost" :
                        mode == SV_BIRTH_MODE_SOLOIST ? "Soloist" :
                        mode == SV_BIRTH_MODE_HELLISH ? "Hellish" :
                        mode == SV_BIRTH_MODE_EVERLASTING ? "Everlasting" :
                        mode == SV_BIRTH_MODE_PVP ? "PvP" :
                        mode == SV_BIRTH_MODE_IDDC ? "IDDC" :
                        mode == SV_BIRTH_MODE_IDDC_SOLOIST ? "IDDC Soloist" :
                        mode == SV_BIRTH_MODE_IDDC_HELLISH ? "Hellish IDDC" :
                        "Dedicated PvP";
                    if (!line(renderer,font,title,60,340 + (int)i * 40,scale,
                              input->birth_view->highlighted_mode == mode ?
                              selected : normal)) return false;
                }
            }
        }
        if (input->login_view && input->login_view->live_handoff) {
            if (!line(renderer,font,"Startup screens complete",40,280,scale,selected) ||
                !line(renderer,font,"Waiting for live-session handoff (Stage C).",40,325,scale,normal) ||
                !line(renderer,font,"Gameplay input, map and HUD are not active.",40,365,scale,normal))
                return false;
        }
    }
    return true;
}

bool sv_endpoint_draw(SDL_Renderer *renderer, SvFont *font,
                      const SvEndpoint *endpoint, const SvEndpointInput *input)
{
    return sv_endpoint_render(renderer,font,endpoint,input) && SDL_RenderPresent(renderer);
}
