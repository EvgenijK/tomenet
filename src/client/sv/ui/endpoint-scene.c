#include "ui/endpoint-scene.h"
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
                        const SvEndpoint *endpoint, const SvEndpointInput *input,
                        float user_scale)
{
    int width, height;
    if (!SDL_GetCurrentRenderOutputSize(renderer, &width, &height)) return false;
    float scale = SDL_GetWindowDisplayScale(SDL_GetRenderWindow(renderer)) * user_scale;
    if (scale <= 0) scale = 1.0f;
    (void)height;
    const SDL_Color title = {226,235,245,255}, normal = {185,205,223,255},
                    selected = {255,210,106,255}, error = {255,135,135,255};
    if (!SDL_SetRenderDrawColor(renderer, 15,21,28,255) || !SDL_RenderClear(renderer)) return false;
    if (!line(renderer,font,"TomeNET SV - Server",40,34,scale,title)) return false;
    if (endpoint->phase == SV_ENDPOINT_LIST) {
        if (!line(renderer,font,"Select a server, or press Q for manual address. Escape exits.",40,85,scale,normal)) return false;
        if (!endpoint->server_count &&
            !line(renderer,font,"No server list available. Press Q to enter a host.",40,130,scale,error)) return false;
        size_t first = endpoint->selected < endpoint->server_count && endpoint->selected >= 20 ?
            endpoint->selected - 19 : 0;
        for (size_t i = first; i < endpoint->server_count && i < first + 20; ++i) {
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
    }
    return true;
}

bool sv_endpoint_draw(SDL_Renderer *renderer, SvFont *font,
                      const SvEndpoint *endpoint, const SvEndpointInput *input,
                      float user_scale)
{
    return sv_endpoint_render(renderer,font,endpoint,input,user_scale) && SDL_RenderPresent(renderer);
}
