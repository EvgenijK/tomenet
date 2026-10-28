#include "ui/endpoint-scene.h"
#include "session/login-view.h"
#include "input/birth-interaction.h"
#include "input/native-birth.h"
#include "ui/login-layout.h"
#include "ui/login-shell.h"
#include "input/credentials.h"
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

static const SDL_Color b2_gold={201,170,77,255}, b2_normal={238,234,221,255},
    b2_muted={133,141,131,255}, b2_error={238,134,118,255};
static bool b2_box(SDL_Renderer *r,SvLoginRect rect,float scale,SDL_Color color,bool outline)
{
    SDL_FRect box={rect.x*scale,rect.y*scale,rect.w*scale,rect.h*scale};
    return SDL_SetRenderDrawColor(r,color.r,color.g,color.b,color.a) &&
        (outline?SDL_RenderRect(r,&box):SDL_RenderFillRect(r,&box));
}
static bool b2_text(SDL_Renderer *r,const char *text,float x,float y,float size,float scale,SDL_Color color)
{ return sv_login_shell_text(r,text,x,y,size,scale,color); }
static bool b2_button(SDL_Renderer *r,SvLoginRect box,const char *text,float scale,bool focused,bool enabled)
{
    return b2_box(r,box,scale,(SDL_Color){31,30,13,220},false) &&
        b2_box(r,box,scale,focused?b2_gold:(SDL_Color){105,95,65,140},true) &&
        b2_text(r,text,box.x+(box.w-strlen(text)*7.2f)/2,box.y+(box.h-12)/2,12,scale,enabled?b2_gold:b2_muted);
}
static bool b2_primary_button(SDL_Renderer *r,SvLoginRect box,const char *text,float scale,bool focused)
{
    return b2_box(r,box,scale,b2_gold,false) &&
        b2_box(r,box,scale,focused?b2_normal:(SDL_Color){130,111,62,255},true) &&
        b2_text(r,text,box.x+(box.w-strlen(text)*7.2f)/2,box.y+(box.h-12)/2,
                12,scale,(SDL_Color){7,9,7,255});
}
/* Draft rendering deliberately bypasses the shell's retained text cache. The
 * password is borrowed only for this draw; transformed bytes are wiped. */
static bool b2_field(SDL_Renderer *r,SvFont *font,SvLoginRect box,const char *bytes,
                     size_t cursor,size_t anchor,float scale,bool active,bool secret)
{
    if(!b2_box(r,box,scale,(SDL_Color){2,5,3,245},false) ||
       !b2_box(r,box,scale,active?b2_gold:(SDL_Color){105,95,65,180},true))return false;
    float cell=sv_font_cell_width(font,scale)/scale;
    size_t visible=(size_t)((box.w-22)/cell),length=strlen(bytes);
    size_t start=cursor>=visible?cursor-visible+1:0;
    if(start>length)start=length;
    size_t count=length-start;if(count>visible)count=visible;
    char draft[SV_TEXT_CAPACITY+1];
    for(size_t i=0;i<count;++i) {
        unsigned char c=(unsigned char)bytes[start+i];
        draft[i]=c>=32 && c<=126?(char)c:'?';
    }
    draft[count]=0;
    size_t lo=cursor<anchor?cursor:anchor,hi=cursor>anchor?cursor:anchor;
    if(lo<start)lo=start;
    if(hi>start+count)hi=start+count;
    bool ok=true;
    if(active && lo<hi)ok=b2_box(r,(SvLoginRect){box.x+10+(lo-start)*cell,box.y+6,(hi-lo)*cell,26},scale,(SDL_Color){79,66,26,255},false);
    if(ok && count)ok=sv_font_draw(font,r,draft,(int)((box.x+10)*scale),(int)((box.y+8)*scale),scale,b2_normal);
    if(active && ok)ok=b2_box(r,(SvLoginRect){box.x+10+(cursor-start)*cell,box.y+7,1,24},scale,b2_gold,false);
    (void)secret;
    volatile char *wipe=draft;for(size_t i=0;i<sizeof(draft);++i)wipe[i]=0;
    return ok;
}
static bool b2_status(SDL_Renderer *r,const char *message,SvLoginRect box,float scale,bool error)
{
    if(!message)return true;
    /* Fixed font size, wrapped within the status region instead of shrinking. */
    size_t columns=(size_t)(box.w/7.3f);if(columns<1)columns=1;
    size_t at=0,length=strlen(message);float y=box.y;
    while(at<length && y+15<=box.y+box.h) {
        size_t count=length-at;if(count>columns)count=columns;
        if(at+count<length) {size_t split=count;while(split && message[at+split]!=' ')--split;if(split)count=split;}
        char row[180];if(count>=sizeof(row))count=sizeof(row)-1;
        memcpy(row,message+at,count);row[count]=0;
        if(!b2_text(r,row,box.x,y,12,scale,error?b2_error:b2_muted))return false;
        at+=count;while(message[at]==' ')++at;y+=18;
    }
    return true;
}
static bool b2_forms(SDL_Renderer *r,SvFont *font,const SvEndpoint *endpoint,const SvEndpointInput *input)
{
    if(!sv_login_shell_render(r,endpoint,input))return false;
    int width,height;if(!SDL_GetCurrentRenderOutputSize(r,&width,&height))return false;
    float scale=SDL_GetWindowDisplayScale(SDL_GetRenderWindow(r));if(scale<=0)scale=1;
    SvLoginLayout l=sv_login_layout(width/scale,height/scale,endpoint->server_count,endpoint->selected);
    SvLoginFormLayout f=sv_login_form_layout(l,height/scale);
    if(endpoint->phase==SV_ENDPOINT_MANUAL) {
        if(!b2_box(r,l.panel,scale,(SDL_Color){0,0,0,180},false) ||
           !b2_box(r,f.modal,scale,(SDL_Color){5,9,6,255},false) || !b2_box(r,f.modal,scale,b2_gold,true) ||
           !sv_login_shell_heading(r,"Direct connection",f.modal.x+19,f.modal.y+18,16,scale,b2_normal) ||
           !b2_text(r,"Enter a server hostname or IP address.",f.modal.x+19,f.modal.y+48,11,scale,b2_muted) ||
           !b2_text(r,"Server address",f.address.x,f.address.y-22,12,scale,b2_gold) ||
           !b2_field(r,font,f.address,endpoint->editor.bytes,endpoint->editor.cursor,endpoint->editor.anchor,scale,input->manual_focus==0,false) ||
           !b2_button(r,f.cancel,"Cancel",scale,input->manual_focus==1,true) ||
           !b2_primary_button(r,f.connect,"Connect",scale,input->manual_focus==2))return false;
        const char *message=endpoint->invalid_address?"Invalid host or port. Edit and try again.":
            input->text_error==SV_TEXT_ENCODING_ERROR?"Cannot encode that server field byte.":
            input->clipboard_unavailable?"Clipboard unavailable; address unchanged.":
            endpoint->editor.searching?(endpoint->editor.preview[0]?endpoint->editor.preview:"No history match"):
            "Hostname or IP, optional :port";
        return b2_status(r,message,(SvLoginRect){f.address.x,f.address.y+48,f.address.w,48},scale,endpoint->invalid_address);
    }
    char server[180];SDL_snprintf(server,sizeof(server),"Logging in %.79s",endpoint->host);
    if(!sv_login_shell_heading(r,server,l.panel.x+20,l.panel.y+9,19,scale,b2_gold) ||
       !b2_text(r,"Use an existing TomeNET account on the selected server.",l.panel.x+20,l.panel.y+33,9,scale,b2_muted) ||
       !b2_box(r,(SvLoginRect){l.panel.x,l.panel.y+51,l.panel.w,1},scale,(SDL_Color){105,95,65,140},false) ||
       !b2_box(r,(SvLoginRect){l.panel.x,l.panel.y+l.panel.h-46,l.panel.w,1},scale,(SDL_Color){105,95,65,140},false))return false;
    if(input->credentials) {
        SvCredentialPresentation view;sv_credentials_view(input->credentials,&view);
        if(!b2_text(r,"Welcome! In order to play, you need to create an account.",l.panel.x+(l.panel.w-54*7.2f)/2,l.panel.y+72,12,scale,b2_muted) ||
           !b2_text(r,"If you don't have an account yet, just enter one of your choice. Remember",l.panel.x+(l.panel.w-73*7.2f)/2,l.panel.y+91,12,scale,b2_muted) ||
           !b2_text(r,"your name and password. Players may only own one account each at a time.",l.panel.x+(l.panel.w-71*7.2f)/2,l.panel.y+110,12,scale,b2_muted) ||
           !b2_text(r,"Name",f.account.x,f.account.y-22,12,scale,b2_gold) ||
           !b2_field(r,font,f.account,view.account,view.cursor[0],view.anchor[0],scale,view.focus==0,false) ||
           !b2_text(r,"Password",f.password.x,f.password.y-22,12,scale,b2_gold))return false;
        SvLoginRect password=f.password;password.w-=66;
        const char *private_password=sv_credentials_private_password(input->credentials);
        if(!b2_field(r,font,password,private_password?private_password:view.password_mask,view.cursor[1],view.anchor[1],scale,view.focus==1,true) ||
           !b2_button(r,f.reveal,view.reveal?"Hide":"Show",scale,view.focus==2,true) ||
           !b2_button(r,f.submit,"LOG IN",scale,view.focus==3,true) ||
           !b2_button(r,f.back,"BACK",scale,view.focus==4,true) ||
           !b2_text(r,"Name and password are case-sensitive! Start name upper-case.",f.account.x,f.submit.y+50,10,scale,b2_muted) ||
           !b2_text(r,"Allowed symbols: .,-'&_$%~#",f.account.x,f.submit.y+65,10,scale,b2_muted) ||
           !b2_text(r,"Guide: unavailable",f.account.x,f.submit.y+96,10,scale,b2_muted))return false;
        if(!l.compact && !sv_login_shell_torches(r,l.panel.x+24,l.panel.x+l.panel.w-48,f.account.y-16,scale))return false;
        const char *message=view.incompatible_password?"Password contains '*' (unsupported by server).":
            view.text_error==SV_TEXT_ENCODING_ERROR?"Cannot encode that credential field byte.":
            view.text_error==SV_TEXT_TRUNCATED?"Field is limited to 15 server bytes.":
            view.clipboard_unavailable?"Clipboard unavailable; credential draft unchanged.":
            view.lookup_pending?"Looking up saved password...":view.vault_status?view.vault_status:"Password is saved only after server confirmation.";
        return b2_status(r,message,f.status,scale,view.incompatible_password || view.text_error==SV_TEXT_ENCODING_ERROR);
    }
    if(!b2_text(r,input->contact_failed?"Connection stopped":"Connecting...",f.account.x,f.account.y,20,scale,input->contact_failed?b2_error:b2_gold) ||
       !b2_status(r,input->contact_status,(SvLoginRect){f.account.x,f.account.y+48,f.account.w,130},scale,input->contact_failed) ||
       !b2_button(r,f.back,"Cancel",scale,false,true))return false;
    if(input->contact_failed && !b2_button(r,f.submit,"Retry",scale,false,input->retry_available))return false;
    return true;
}

bool sv_endpoint_render(SDL_Renderer *renderer, SvFont *font,
                        const SvEndpoint *endpoint, const SvEndpointInput *input)
{
    if (endpoint->phase == SV_ENDPOINT_LIST && !input->temporary_surface_text)
        return sv_login_shell_render(renderer, endpoint,input);
    if (!input->temporary_surface_text && (endpoint->phase == SV_ENDPOINT_MANUAL ||
        (endpoint->phase == SV_ENDPOINT_SELECTED && (input->credentials || input->contact_failed ||
         !input->login_view || (!input->login_view->overview && !input->login_view->birth && !input->login_view->motd)))))
        return b2_forms(renderer,font,endpoint,input);
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
            if (!input->login_view->ordinary_name_entry &&
                input->login_view->ordinary_creation_available &&
                !line(renderer,font,"N) Create a new character",40,225,scale,normal))
                return false;
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
                if (!line(renderer,font,"Choose sex (* random, # DNA, % automatic DNA, Q quits)",40,270,
                          scale,normal) ||
                    !line(renderer,font,"Male",60,310,scale,
                          input->birth_view->highlighted_sex == SV_BIRTH_SEX_MALE ?
                          selected : normal) ||
                    !line(renderer,font,"Female",60,360,scale,
                          input->birth_view->highlighted_sex == SV_BIRTH_SEX_FEMALE ?
                          selected : normal)) return false;
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_CLASS) {
                if (!line(renderer,font,"Choose class (* random, # DNA, % automatic DNA, Q quits)",40,290,
                          scale,normal)) return false;
                unsigned first = input->birth_viewport ?
                    input->birth_viewport->first : 0;
                unsigned rows = input->birth_viewport ?
                    input->birth_viewport->rows : input->birth_view->visible_class_count;
                for (unsigned i = first; i < input->birth_view->visible_class_count &&
                     i < first + rows; ++i) {
                    unsigned id = input->birth_view->visible_classes[i];
                    bool legal = false;
                    for (unsigned j = 0;
                         j < input->birth_view->legal_class_count; ++j)
                        if (input->birth_view->legal_classes[j] == id) legal = true;
                    if (!line(renderer,font,input->birth_view->setup->classes[id].title,
                              60,SV_BIRTH_CHOICE_ROW_TOP + SV_BIRTH_CHOICE_TEXT_INSET +
                              (int)(i - first) * SV_BIRTH_CHOICE_ROW_HEIGHT,scale,
                              input->birth_view->highlighted_class == (int)id ?
                              selected : legal ? normal : disabled)) return false;
                }
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_RACE) {
                if (!line(renderer,font,"Choose race (* random, # DNA, % automatic DNA, Q quits)",40,290,
                          scale,normal)) return false;
                unsigned first = input->birth_viewport ?
                    input->birth_viewport->first : 0;
                unsigned rows = input->birth_viewport ?
                    input->birth_viewport->rows : input->birth_view->setup->race_count;
                for (unsigned id = first; id < input->birth_view->setup->race_count &&
                     id < first + rows; ++id) {
                    bool legal = false;
                    for (unsigned i = 0; i < input->birth_view->legal_race_count; ++i)
                        if (input->birth_view->legal_races[i] == id) legal = true;
                    if (!line(renderer,font,input->birth_view->setup->races[id].title,
                              60,SV_BIRTH_CHOICE_ROW_TOP + SV_BIRTH_CHOICE_TEXT_INSET +
                              (int)(id - first) * SV_BIRTH_CHOICE_ROW_HEIGHT,scale,
                              input->birth_view->highlighted_race == (int)id ?
                              selected : legal ? normal : disabled)) return false;
                }
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_TRAIT) {
                if (!line(renderer,font,"Choose trait (* random, # DNA, % automatic DNA, Q quits)",40,290,
                          scale,normal)) return false;
                unsigned first = input->birth_viewport ?
                    input->birth_viewport->first : 0;
                unsigned rows = input->birth_viewport ?
                    input->birth_viewport->rows : input->birth_view->legal_trait_count;
                for (unsigned i = first; i < input->birth_view->legal_trait_count &&
                     i < first + rows; ++i) {
                    unsigned id = input->birth_view->legal_traits[i];
                    if (!line(renderer,font,input->birth_view->setup->traits[id].title,
                              60,SV_BIRTH_CHOICE_ROW_TOP + SV_BIRTH_CHOICE_TEXT_INSET +
                              (int)(i - first) * SV_BIRTH_CHOICE_ROW_HEIGHT,scale,
                              input->birth_view->highlighted_trait == (int)id ?
                              selected : normal)) return false;
                }
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_BODY) {
                if (!line(renderer,font,"Choose body (* random, # DNA, % automatic DNA, Q quits)",40,290,
                          scale,normal)) return false;
                unsigned first = input->birth_viewport ?
                    input->birth_viewport->first : 0;
                unsigned rows = input->birth_viewport ?
                    input->birth_viewport->rows : input->birth_view->legal_body_count;
                for (unsigned i = first; i < input->birth_view->legal_body_count &&
                     i < first + rows; ++i) {
                    SvBirthBody body = input->birth_view->legal_bodies[i];
                    const char *title = body == SV_BIRTH_BODY_FRUIT_BAT ?
                        "Fruit bat" : "Normal body";
                    if (!line(renderer,font,title,60,
                              SV_BIRTH_CHOICE_ROW_TOP + SV_BIRTH_CHOICE_TEXT_INSET +
                              (int)(i - first) * SV_BIRTH_CHOICE_ROW_HEIGHT,scale,
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
                             "Distribute stats (%d points left, # DNA, %% automatic DNA, Q quits)",
                             input->birth_view->stats_remaining);
                else snprintf(prompt, sizeof(prompt),
                              "Choose stat order (* random, Q quits)");
                if (!line(renderer,font,prompt,40,290,scale,normal)) return false;
                unsigned first = input->birth_viewport ?
                    input->birth_viewport->first : 0;
                unsigned rows = input->birth_viewport ?
                    input->birth_viewport->rows : input->birth_view->legal_stat_count;
                for (unsigned i = first; i < input->birth_view->legal_stat_count &&
                     i < first + rows; ++i) {
                    unsigned stat = input->birth_view->legal_stats[i];
                    char row[96];
                    if (input->birth_view->stats_point_buy)
                        snprintf(row, sizeof(row), "%s: %d", names[stat],
                                 input->birth_view->stats[stat]);
                    else snprintf(row, sizeof(row), "%s", names[stat]);
                    if (!line(renderer,font,row,60,
                              SV_BIRTH_CHOICE_ROW_TOP + SV_BIRTH_CHOICE_TEXT_INSET +
                              (int)(i - first) * SV_BIRTH_CHOICE_ROW_HEIGHT,scale,
                              input->birth_view->highlighted_stat == (int)stat ?
                              selected : normal)) return false;
                }
            } else if (input->birth_view && input->birth_view->owner == SV_BIRTH_MODE) {
                if (!line(renderer,font,"Choose mode (* random, # DNA, % automatic DNA, Q quits)",40,290,
                          scale,normal)) return false;
                unsigned first = input->birth_viewport ?
                    input->birth_viewport->first : 0;
                unsigned rows = input->birth_viewport ?
                    input->birth_viewport->rows : input->birth_view->legal_mode_count;
                for (unsigned i = first; i < input->birth_view->legal_mode_count &&
                     i < first + rows; ++i) {
                    SvBirthMode mode = input->birth_view->legal_modes[i];
                    const char *title = mode == SV_BIRTH_MODE_NORMAL ? "Normal" :
                        mode == SV_BIRTH_MODE_NO_GHOST ? "No Ghost" :
                        mode == SV_BIRTH_MODE_SOLOIST ? "Soloist" :
                        mode == SV_BIRTH_MODE_HELLISH ? "Hellish" :
                        mode == SV_BIRTH_MODE_EVERLASTING ? "Everlasting" :
                        mode == SV_BIRTH_MODE_PVP ? "PvP" :
                        mode == SV_BIRTH_MODE_FRUIT_BAT ? "Fruit bat" :
                        mode == SV_BIRTH_MODE_IDDC ? "IDDC" :
                        mode == SV_BIRTH_MODE_IDDC_SOLOIST ? "IDDC Soloist" :
                        mode == SV_BIRTH_MODE_IDDC_HELLISH ? "Hellish IDDC" :
                        "Dedicated PvP";
                    if (!line(renderer,font,title,60,
                              SV_BIRTH_CHOICE_ROW_TOP + SV_BIRTH_CHOICE_TEXT_INSET +
                              (int)(i - first) * SV_BIRTH_CHOICE_ROW_HEIGHT,scale,
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
    if(!sv_endpoint_render(renderer,font,endpoint,input))return false;
    const char *capture=SDL_getenv("SV_B2_CAPTURE");
    /* Capture complete production frames only while private fields are masked.
     * Reuse the shell capture hook after overlays, rather than its base layer. */
    if(capture && (!input->credentials || !input->credentials->reveal)) {
        SDL_PropertiesID properties=SDL_GetRendererProperties(renderer);
        const char *counter="sv.login.capture.frame";
        Sint64 frame=SDL_GetNumberProperty(properties,counter,0)+1;
        if(!SDL_SetNumberProperty(properties,counter,frame))return false;
        const char *delay=SDL_getenv("SV_B2_CAPTURE_FRAME");
        int target=delay?SDL_atoi(delay):1;if(target<1)target=1;
        if(frame==target) {
            SDL_Surface *surface=SDL_RenderReadPixels(renderer,NULL);
            if(!surface)return false;
            bool saved=SDL_SaveBMP(surface,capture);SDL_DestroySurface(surface);
            if(!saved)return false;
        }
    }
    return SDL_RenderPresent(renderer);
}
