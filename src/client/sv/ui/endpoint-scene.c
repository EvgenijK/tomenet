#include "ui/endpoint-scene.h"
#include "session/login-view.h"
#include "input/birth-interaction.h"
#include "input/native-birth.h"
#include "ui/login-layout.h"
#include "ui/login-shell.h"
#include "ui/birth-descriptions.h"
#include "input/credentials.h"
#include <stdio.h>
#include <string.h>

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
/* Query current window coordinates so hover follows resize and surface changes
 * without retaining a second geometry or changing keyboard focus. */
static bool b2_hover(SDL_Renderer *r,SvLoginRect box,float scale)
{
    SDL_Window *window=SDL_GetRenderWindow(r);
    if(SDL_GetMouseFocus()!=window || scale<=0)return false;
    float x,y,density=SDL_GetWindowPixelDensity(window);
    SDL_GetMouseState(&x,&y);
    return density>0 && sv_login_contains(box,x*density/scale,y*density/scale);
}
static bool b2_button(SDL_Renderer *r,SvLoginRect box,const char *text,float scale,bool focused,bool enabled)
{
    bool hover=enabled && b2_hover(r,box,scale);
    return b2_box(r,box,scale,(SDL_Color){31,30,13,220},false) &&
        b2_box(r,box,scale,focused?b2_gold:hover?(SDL_Color){157,140,89,255}:(SDL_Color){105,95,65,140},true) &&
        b2_text(r,text,box.x+(box.w-strlen(text)*7.2f)/2,box.y+(box.h-12)/2,12,scale,enabled?b2_gold:b2_muted);
}
static bool b2_primary_button(SDL_Renderer *r,SvLoginRect box,const char *text,float scale,bool focused)
{
    return b2_box(r,box,scale,b2_hover(r,box,scale)?(SDL_Color){222,194,107,255}:b2_gold,false) &&
        b2_box(r,box,scale,focused?b2_normal:(SDL_Color){130,111,62,255},true) &&
        b2_text(r,text,box.x+(box.w-strlen(text)*7.2f)/2,box.y+(box.h-12)/2,
                12,scale,(SDL_Color){7,9,7,255});
}
static bool b2_round(SDL_Renderer *r,SvLoginRect b,float radius,float scale,SDL_Color color,bool outline)
{ return sv_login_shell_roundrect(r,(SDL_FRect){b.x,b.y,b.w,b.h},radius,scale,color,outline); }
static bool b2_center(SDL_Renderer *r,const char *value,SvLoginRect region,float y,float size,float scale,SDL_Color color)
{
    float width;
    return sv_login_shell_measure(r,value,size,scale,false,&width) &&
        b2_text(r,value,region.x+(region.w-width)/2,y,size,scale,color);
}
/* The URL's logical width also defines its mouse hit rectangle. Keep the
 * reference 10px monospace advance stable under fractional-DPI font hinting. */
static bool b2_account_guide(SDL_Renderer *r,SvLoginRect guide,float scale)
{
    const char *url="https://www.tomenet.eu/guide.php";
    float actual,sx,sy;
    if(!sv_login_shell_measure(r,url,10,scale,false,&actual) || actual<=0 ||
       !SDL_GetRenderScale(r,&sx,&sy))return false;
    float factor=guide.w/actual;
    if(!SDL_SetRenderScale(r,sx*factor,sy))return false;
    bool ok=b2_text(r,url,guide.x/factor,guide.y,10,scale,
        b2_hover(r,guide,scale)?(SDL_Color){239,211,109,255}:(SDL_Color){181,158,81,255});
    bool restored=SDL_SetRenderScale(r,sx,sy);
    return ok && restored && b2_box(r,(SvLoginRect){guide.x,guide.y+12,guide.w,1},scale,(SDL_Color){181,158,81,255},false);
}
/* Game action controls use the final D 10px uppercase role with 1px tracking. */
static bool b2_account_action(SDL_Renderer *r,SvLoginRect b,const char *value,float scale,bool focus,bool secondary)
{
    bool hover=b2_hover(r,b,scale);
    SDL_Color base=secondary?(SDL_Color){7,9,7,255}:hover?(SDL_Color){16,20,14,255}:(SDL_Color){8,11,8,255};
    if(!b2_box(r,b,scale,base,false))return false;
    if(!secondary || hover) {
        SDL_FColor gold=hover?(SDL_FColor){183/255.f,143/255.f,58/255.f,.16f}:(SDL_FColor){141/255.f,112/255.f,55/255.f,.1f};
        SDL_FColor clear=gold;clear.a=0;
        SDL_Vertex vertices[]={
            {.position={b.x*scale,b.y*scale},.color=gold},
            {.position={(b.x+b.w)*scale,b.y*scale},.color=gold},
            {.position={(b.x+b.w)*scale,(b.y+b.h*.72f)*scale},.color=clear},
            {.position={b.x*scale,(b.y+b.h*.72f)*scale},.color=clear}};
        const int indices[]={0,1,2,0,2,3};
        if(!SDL_RenderGeometry(r,NULL,vertices,4,indices,6))return false;
    }
    SDL_Color border=hover?(SDL_Color){177,139,61,255}:secondary?(SDL_Color){78,66,40,255}:(SDL_Color){141,113,56,255};
    if(!b2_box(r,b,scale,border,true))return false;
    if(focus && !b2_box(r,(SvLoginRect){b.x-3,b.y-3,b.w+6,b.h+6},scale,(SDL_Color){211,171,80,255},true))return false;
    float total=0;
    for(size_t i=0;value[i];++i) {char glyph[2]={value[i],0};float width;
        if(!sv_login_shell_measure(r,glyph,10,scale,false,&width))return false;total+=width+1;}
    if(total)total-=1;
    float x=b.x+(b.w-total)/2;
    for(size_t i=0;value[i];++i) {char glyph[2]={value[i],0};float width;
        if(!sv_login_shell_measure(r,glyph,10,scale,false,&width) ||
           !b2_text(r,glyph,x,b.y+(b.h-12)/2,10,scale,hover?(SDL_Color){241,207,93,255}:
                    secondary?(SDL_Color){170,169,159,255}:(SDL_Color){228,196,83,255}))return false;
        x+=width+1;
    }
    return true;
}
/* Draft rendering deliberately bypasses the shell's retained text cache. The
 * password is borrowed only for this draw; transformed bytes are wiped. */
static bool b2_field_style(SDL_Renderer *r,SvFont *font,SvLoginRect box,const char *bytes,
                     size_t cursor,size_t anchor,float scale,bool active,bool secret,bool account_style)
{
    if(!account_style) {
        if(!b2_box(r,box,scale,(SDL_Color){2,5,3,245},false) ||
           !b2_box(r,box,scale,active?b2_gold:b2_hover(r,box,scale)?(SDL_Color){157,140,89,255}:(SDL_Color){105,95,65,180},true))return false;
    } else {
    if(!b2_round(r,box,6,scale,(SDL_Color){1,4,2,209},false) ||
       !b2_round(r,box,6,scale,(SDL_Color){70,74,65,255},true))return false;
    if(active && (!b2_round(r,(SvLoginRect){box.x-3,box.y-3,box.w+6,box.h+6},9,scale,(SDL_Color){224,186,75,255},true) ||
                  !b2_round(r,(SvLoginRect){box.x-2,box.y-2,box.w+4,box.h+4},8,scale,(SDL_Color){224,186,75,255},true)))return false;
    }
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
static bool b2_field(SDL_Renderer *r,SvFont *font,SvLoginRect box,const char *bytes,
                     size_t cursor,size_t anchor,float scale,bool active,bool secret)
{ return b2_field_style(r,font,box,bytes,cursor,anchor,scale,active,secret,false); }
static bool b2_account_field(SDL_Renderer *r,SvFont *font,SvLoginRect box,const char *bytes,
                     size_t cursor,size_t anchor,float scale,bool active,bool secret)
{ return b2_field_style(r,font,box,bytes,cursor,anchor,scale,active,secret,true); }
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
/* Shared visual frame only: actions and dismissal stay with each caller. */
static bool b2_state_frame(SDL_Renderer *r,SvLoginRect shade,SvLoginRect card,
                           const char *title,float scale,bool error)
{
    return b2_box(r,shade,scale,(SDL_Color){0,0,0,190},false) &&
        b2_box(r,card,scale,(SDL_Color){7,11,7,255},false) &&
        b2_box(r,card,scale,error?b2_error:b2_gold,true) &&
        sv_login_shell_heading(r,title,card.x+22,card.y+22,22,scale,error?b2_error:b2_gold);
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
    float title_width;
    if(!sv_login_shell_measure(r,server,19,scale,true,&title_width) ||
       !b2_box(r,(SvLoginRect){l.panel.x+1,l.panel.y+1,l.panel.w-2,50},scale,(SDL_Color){30,30,24,174},false) ||
       !sv_login_shell_heading(r,server,l.panel.x+20,l.panel.y+7,19,scale,(SDL_Color){239,208,105,255}) ||
       !b2_text(r,"Use an existing TomeNET account on the selected server.",l.panel.x+20,l.panel.y+32,9,scale,b2_muted) ||
       !b2_box(r,(SvLoginRect){l.panel.x,l.panel.y+51,l.panel.w,1},scale,(SDL_Color){48,42,27,255},false) ||
       !b2_box(r,(SvLoginRect){l.panel.x+1,l.panel.y+l.panel.h-45,l.panel.w-2,44},scale,(SDL_Color){4,7,5,184},false) ||
       !b2_box(r,(SvLoginRect){l.panel.x,l.panel.y+l.panel.h-46,l.panel.w,1},scale,(SDL_Color){48,42,27,255},false))return false;
    /* The h2 ornament from theme.css occupies the remaining heading flex item. */
    float rule_x=l.panel.x+20+title_width+8,rule_w=SDL_min(48,l.panel.x+l.panel.w-20-rule_x);
    if(rule_w>0) {
        SDL_Vertex v[]={
            {.position={rule_x*scale,(l.panel.y+18)*scale},.color={118/255.f,95/255.f,52/255.f,1}},
            {.position={(rule_x+rule_w)*scale,(l.panel.y+18)*scale},.color={118/255.f,95/255.f,52/255.f,0}},
            {.position={(rule_x+rule_w)*scale,(l.panel.y+19)*scale},.color={118/255.f,95/255.f,52/255.f,0}},
            {.position={rule_x*scale,(l.panel.y+19)*scale},.color={118/255.f,95/255.f,52/255.f,1}}};
        const int indices[]={0,1,2,0,2,3};if(!SDL_RenderGeometry(r,NULL,v,4,indices,6))return false;
    }
    if(input->credentials) {
        SvCredentialPresentation view;sv_credentials_view(input->credentials,&view);
        if(!b2_center(r,"Welcome! In order to play, you need to create an account.",l.panel,l.panel.y+72,12,scale,b2_muted) ||
           !b2_center(r,"If you don't have an account yet, just enter one of your choice. Remember",l.panel,l.panel.y+91,12,scale,b2_muted) ||
           !b2_center(r,"your name and password. Players may only own one account each at a time.",l.panel,l.panel.y+110,12,scale,b2_muted) ||
           !b2_text(r,"Name",f.account.x,f.account.y-22,11,scale,(SDL_Color){168,169,159,255}) ||
           !b2_account_field(r,font,f.account,view.account,view.cursor[0],view.anchor[0],scale,view.focus==0,false) ||
           !b2_text(r,"Password",f.password.x,f.password.y-22,11,scale,(SDL_Color){168,169,159,255}))return false;
        SvLoginRect password=f.password;
        const char *private_password=sv_credentials_private_password(input->credentials);
        float reveal_width;
        if(!sv_login_shell_measure(r,view.reveal?"Hide":"Show",16,scale,false,&reveal_width))return false;
        if(!b2_account_field(r,font,password,private_password?private_password:view.password_mask,view.cursor[1],view.anchor[1],scale,view.focus==1,true) ||
           !b2_text(r,view.reveal?"Hide":"Show",f.password.x+f.password.w-10-reveal_width,f.reveal.y+10,16,scale,
                    (view.focus==2 || b2_hover(r,f.reveal,scale))?(SDL_Color){239,211,109,255}:(SDL_Color){154,140,88,255}) ||
           !b2_account_action(r,f.submit,"LOG IN",scale,view.focus==3,false) ||
           !b2_account_action(r,f.account_back,"BACK",scale,view.focus==4,true) ||
           !b2_text(r,"Name and password are case-sensitive! Name must start on upper-case letter.",f.account.x,f.submit.y+47,10,scale,(SDL_Color){119,125,117,255}) ||
           !b2_text(r,"The name may also contain numbers, spaces and these symbols: .,-'&_$%~#",f.account.x,f.submit.y+62.5f,10,scale,(SDL_Color){119,125,117,255}) ||
           !b2_text(r,"If you are new to TomeNET, this guide may prove useful:",f.account.x,f.submit.y+93,10,scale,(SDL_Color){119,125,117,255}) ||
           !b2_account_guide(r,f.guide,scale))return false;
        if(!l.compact && !sv_login_shell_torches(r,l.panel.x+85,l.panel.x+l.panel.w-97,f.account.y-24,scale))return false;
        const char *message=view.incompatible_password?"Password contains '*' (unsupported by server).":
            view.text_error==SV_TEXT_ENCODING_ERROR?"Cannot encode that credential field byte.":
            view.text_error==SV_TEXT_TRUNCATED?"Field is limited to 15 server bytes.":
            view.clipboard_unavailable?"Clipboard unavailable; credential draft unchanged.":
            view.lookup_pending?"Looking up saved password...":view.vault_status;
        return b2_status(r,message,f.status,scale,view.incompatible_password || view.text_error==SV_TEXT_ENCODING_ERROR);
    }
    SvLoginRect card={f.account.x-22,f.account.y-22,f.account.w+44,244};
    if(!b2_state_frame(r,l.panel,card,input->contact_failed?"Connection stopped":"Connecting...",scale,input->contact_failed) ||
       !b2_status(r,input->contact_status,(SvLoginRect){f.account.x,f.account.y+48,f.account.w,130},scale,input->contact_failed) ||
       !b2_button(r,f.back,"Cancel",scale,false,true))return false;
    if(input->contact_failed && !b2_button(r,f.submit,"Retry",scale,false,input->retry_available))return false;
    return true;
}

/* Character overview typography stays inside its reference grid tracks. */
static bool b2_character_text(SDL_Renderer *r,const char *value,SvLoginRect box,
                              float size,float tracking,float scale,SDL_Color color,
                              bool heading,bool centered,bool right)
{
    if(!value[0] || box.w<=0 || box.h<=0)return true;
    char text[256];SDL_strlcpy(text,value,sizeof(text));
    float width=0;
    for(;;) {
        if(!sv_login_shell_measure(r,text,size,scale,heading,&width))return false;
        width+=tracking*(strlen(text)?strlen(text)-1:0);
        if(width<=box.w || !text[0])break;
        size_t length=strlen(text);
        if(length>=3 && !strcmp(text+length-3,"..."))length-=3;
        if(!length){text[0]=0;break;}
        --length;while(length && ((unsigned char)text[length]&0xc0)==0x80)--length;
        text[length]=0;SDL_strlcat(text,"...",sizeof(text));
    }
    if(!text[0])return true;
    SDL_Rect previous,clip={(int)(box.x*scale),(int)(box.y*scale),(int)(box.w*scale),(int)(box.h*scale)};
    bool clipped=SDL_RenderClipEnabled(r);
    if(!SDL_GetRenderClipRect(r,&previous))return false;
    if(clipped) {
        SDL_Rect intersection;
        if(!SDL_GetRectIntersection(&clip,&previous,&intersection))return true;
        clip=intersection;
    }
    if(!SDL_SetRenderClipRect(r,&clip))return false;
    float x=box.x+(centered?(box.w-width)/2:right?box.w-width:0);
    bool ok=true;
    if(!tracking)ok=heading?sv_login_shell_heading(r,text,x,box.y,size,scale,color):b2_text(r,text,x,box.y,size,scale,color);
    else for(size_t i=0;ok && text[i];++i) {
        char glyph[2]={text[i],0};float advance;
        ok=sv_login_shell_measure(r,glyph,size,scale,false,&advance) && b2_text(r,glyph,x,box.y,size,scale,color);
        x+=advance+tracking;
    }
    return SDL_SetRenderClipRect(r,clipped?&previous:NULL) && ok;
}
static bool b2_character_action(SDL_Renderer *r,SvLoginRect box,const char *text,float scale,
                                 bool focus,bool enabled,bool primary)
{
    if(enabled)return b2_account_action(r,box,text,scale,focus,!primary);
    return b2_box(r,box,scale,(SDL_Color){9,11,9,255},false) &&
        b2_box(r,box,scale,(SDL_Color){52,55,47,255},true) &&
        b2_character_text(r,text,(SvLoginRect){box.x+4,box.y+10,box.w-8,16},10,1,scale,
            (SDL_Color){104,107,100,255},false,true,false);
}
static bool b2_character_diamond(SDL_Renderer *r,float x,float y,float radius,float scale,SDL_Color color,bool fill)
{
    /* Coverage fringes follow the diagonal's normal, so its one-logical-unit
     * border remains even at fractional output scales. */
    const float dx[]={0,1,0,-1},dy[]={-1,0,1,0};
    float aa=.5f/scale,inset[]={-aa,aa,1-aa,1+aa};
    SDL_Vertex vertices[17];int indices[84],count=0,rings=fill?2:4;
    for(int ring=0;ring<rings;++ring) {
        float extent=SDL_max(0,radius-inset[ring]*SDL_sqrtf(2));
        float alpha=ring==0 || (!fill && ring==3)?0:color.a/255.f;
        for(int side=0;side<4;++side)vertices[ring*4+side]=(SDL_Vertex){
            .position={(x+dx[side]*extent)*scale,(y+dy[side]*extent)*scale},
            .color={color.r/255.f,color.g/255.f,color.b/255.f,alpha}};
        if(ring)for(int side=0;side<4;++side) {
            int outer=(ring-1)*4+side,next=(side+1)%4,inner=ring*4+side;
            int quad[]={outer,(ring-1)*4+next,inner,(ring-1)*4+next,ring*4+next,inner};
            for(int k=0;k<6;++k)indices[count++]=quad[k];
        }
    }
    if(fill) {
        vertices[8]=(SDL_Vertex){.position={x*scale,y*scale},
            .color={color.r/255.f,color.g/255.f,color.b/255.f,color.a/255.f}};
        for(int side=0;side<4;++side){indices[count++]=8;indices[count++]=4+side;indices[count++]=4+(side+1)%4;}
    }
    return SDL_RenderGeometry(r,NULL,vertices,rings*4+(fill?1:0),indices,count);
}
static bool b2_character_row_frame(SDL_Renderer *r,SvLoginRect row,float scale,bool selected,bool hover)
{
    if(!b2_round(r,row,8,scale,hover?(SDL_Color){12,16,13,255}:(SDL_Color){7,11,8,194},false))return false;
    if(selected) {
        SDL_Vertex vertices[29];int indices[84];
        vertices[0]=(SDL_Vertex){.position={(row.x+row.w/2)*scale,(row.y+row.h/2)*scale},
            .color={52/255.f,44/255.f,16/255.f,133/255.f}};
        for(int corner=0;corner<4;++corner)for(int step=0;step<=6;++step) {
            float angle=(corner*90+step*15)*SDL_PI_F/180.f;
            float cx=row.x+(corner==0 || corner==3?row.w-8:8),cy=row.y+(corner<2?row.h-8:8);
            float x=cx+8*SDL_cosf(angle),y=cy+8*SDL_sinf(angle),t=(x-row.x)/row.w;
            vertices[1+corner*7+step]=(SDL_Vertex){.position={x*scale,y*scale},
                .color={(87+(17-87)*t)/255.f,(70+(18-70)*t)/255.f,(24+(8-24)*t)/255.f,(82+(184-82)*t)/255.f}};
        }
        for(int i=0;i<28;++i){indices[i*3]=0;indices[i*3+1]=i+1;indices[i*3+2]=i==27?1:i+2;}
        if(!SDL_RenderGeometry(r,NULL,vertices,29,indices,84))return false;
    }
    if(!b2_round(r,row,8,scale,selected?b2_gold:hover?(SDL_Color){114,99,58,255}:(SDL_Color){49,53,45,255},true))return false;
    return true;
}
static bool b2_character_placeholder(SDL_Renderer *r,SvLoginRect row,const char *label,float scale,bool dedicated)
{
    SDL_Color border=dedicated?(SDL_Color){56,58,50,255}:(SDL_Color){48,53,46,255};
    if(!b2_round(r,row,8,scale,(SDL_Color){5,8,6,97},false))return false;
    for(float x=8;x<row.w-8;x+=5) {
        float length=SDL_min(2,row.w-8-x);
        if(!b2_box(r,(SvLoginRect){row.x+x,row.y,length,1},scale,border,false) ||
           !b2_box(r,(SvLoginRect){row.x+x,row.y+row.h-1,length,1},scale,border,false))return false;
    }
    for(float y=8;y<row.h-8;y+=5)
        if(!b2_box(r,(SvLoginRect){row.x,row.y+y,1,2},scale,border,false) ||
           !b2_box(r,(SvLoginRect){row.x+row.w-1,row.y+y,1,2},scale,border,false))return false;
    for(int corner=0;corner<4;++corner)for(int segment=0;segment<6;segment+=2) {
        float angle=(corner*90+segment*15)*SDL_PI_F/180.f,next=angle+15*SDL_PI_F/180.f;
        float cx=row.x+(corner==0 || corner==3?row.w-8:8),cy=row.y+(corner<2?row.h-8:8);
        if(!SDL_SetRenderDrawColor(r,border.r,border.g,border.b,255) ||
           !SDL_RenderLine(r,(cx+8*SDL_cosf(angle))*scale,(cy+8*SDL_sinf(angle))*scale,
                            (cx+8*SDL_cosf(next))*scale,(cy+8*SDL_sinf(next))*scale))return false;
    }
    return b2_character_diamond(r,row.x+27,row.y+row.h/2,16.97f,scale,(SDL_Color){56,59,52,255},false) &&
        b2_character_diamond(r,row.x+27,row.y+row.h/2,3,scale,(SDL_Color){85,91,84,255},false) &&
        b2_character_text(r,label,(SvLoginRect){row.x+52,row.y+14,row.w-64,16},11,0,scale,
            dedicated?(SDL_Color){112,110,100,255}:(SDL_Color){102,108,101,255},false,false,false);
}
static void b2_character_mode(char out[32],int16_t raw)
{
    unsigned mode=(uint16_t)raw,base=mode&0xffu;
    const char *label=NULL;
    /* Sex/body and dedicated-slot eligibility are not game-mode labels. */
    if(!(mode&~0x0fdfu))switch(base) {
    case 0:label="NORMAL";break;case 1:label="SOLO";break;
    case 2:label="HARD MODE";break;case 4:label="NO GHOST";break;
    case 5:label="SOLO";break;case 6:label="HELLISH";break;
    case 8:label="EVERLASTING";break;case 16:label="PVP";break;
    case 0x40:case 0x44:label="IDDC";break;case 0x45:label="IDDC SOLO";break;
    case 0x46:label="IDDC HELLISH";break;case 0x80:case 0x90:label="PVP";break;
    }
    if(label)SDL_strlcpy(out,label,32);else SDL_snprintf(out,32,"MODE 0x%04X",mode);
}

static bool b2_level_badge(SDL_Renderer *r,SvLoginRect row,float scale)
{
    float x=row.x+48,y=row.y+row.h/2;
    return b2_character_diamond(r,x,y,15.27f,scale,(SDL_Color){32,28,17,255},true) &&
        b2_character_diamond(r,x,y,15.27f,scale,(SDL_Color){177,138,55,255},false) &&
        b2_character_diamond(r,x,y,13,scale,(SDL_Color){8,9,7,255},false);
}
static bool b2_wrap(SDL_Renderer *r,const char *text,SvLoginRect box,float size,float scale,SDL_Color color);
static bool b2_name_intro(SDL_Renderer *r,SvLoginRect box,float scale,bool short_height)
{
    unsigned columns=(unsigned)(box.w/(15*.61f));if(!columns)columns=1;
    size_t at=0,length=sizeof(sv_name_intro_text)-1;float y=box.y;
    while(at<length) {
        size_t count=length-at;if(count>columns)count=columns;
        if(at+count<length){size_t split=count;while(split && sv_name_intro_text[at+split]!=' ')--split;if(split)count=split;}
        char line[sizeof(sv_name_intro_text)];memcpy(line,sv_name_intro_text+at,count);line[count]=0;
        if(!b2_text(r,line,box.x,y,15,scale,(SDL_Color){133,139,130,255}))return false;
        at+=count;while(sv_name_intro_text[at]==' ')++at;y+=short_height?20.25f:22.5f;
    }
    return true;
}

/* Creation paper is shared by name entry and native birth steps. The initial
 * 10s are a presentation preview; prepared point-buy values stay authoritative. */
static bool b2_creation_value(SDL_Renderer *r,const char *text,SvLoginRect box,float scale)
{
    unsigned columns=(unsigned)(box.w/(15*.61f));if(!columns)columns=1;
    size_t at=0,length=strlen(text);
    while(at<length) {
        size_t count=length-at;if(count>columns)count=columns;
        if(at+count<length) {size_t split=count;while(split && text[at+split]!=' ')--split;if(split)count=split;}
        char line[SV_CONTACT_NAME_CAPACITY];if(count>=sizeof(line))count=sizeof(line)-1;
        memcpy(line,text+at,count);line[count]=0;
        if(!b2_character_text(r,line,box,15,0,scale,(SDL_Color){224,222,202,255},false,false,true))return false;
        at+=count;while(text[at]==' ')++at;box.y+=22.5f;
    }
    return true;
}
static unsigned b2_wrap_count(const char *text,unsigned columns);
static bool b2_creation_summary(SDL_Renderer *r,const char *name,const char *const *values,const char *const *numbers,SvBirthViewport *viewport,SvLoginRect box,float scale,bool short_height)
{
    short_height=short_height || box.h<600;
    int left=(int)SDL_floorf(box.x*scale)-1,top_edge=(int)SDL_floorf(box.y*scale)-1;
    SDL_Rect previous,clip={left,top_edge,(int)SDL_ceilf((box.x+box.w)*scale)+2-left,
        (int)SDL_ceilf((box.y+box.h)*scale)+2-top_edge};
    bool clipped=SDL_RenderClipEnabled(r);
    if(!SDL_GetRenderClipRect(r,&previous))return false;
    if(clipped && !SDL_GetRectIntersection(&clip,&previous,&clip))return true;
    if(!SDL_SetRenderClipRect(r,&clip))return false;
    SDL_Vertex paper[]={
        {.position={box.x*scale,box.y*scale},.color={26/255.f,25/255.f,14/255.f,.92f}},
        {.position={(box.x+box.w)*scale,box.y*scale},.color={3/255.f,8/255.f,5/255.f,.92f}},
        {.position={(box.x+box.w)*scale,(box.y+box.h)*scale},.color={10/255.f,15/255.f,10/255.f,.96f}},
        {.position={box.x*scale,(box.y+box.h)*scale},.color={3/255.f,8/255.f,5/255.f,.92f}}};
    const int quad[]={0,1,2,0,2,3};
    bool ok=SDL_RenderGeometry(r,NULL,paper,4,quad,6);
    SDL_Vertex glow[34];int fan[96];
    float cx=box.x+box.w*.18f,cy=box.y+box.h*.04f,radius=box.w*.95f;
    glow[0]=(SDL_Vertex){.position={cx*scale,cy*scale},.color={218/255.f,178/255.f,69/255.f,.16f}};
    for(int i=0;i<=32;++i) {
        float angle=i*2*SDL_PI_F/32;
        glow[i+1]=(SDL_Vertex){.position={(cx+radius*SDL_cosf(angle))*scale,(cy+radius*SDL_sinf(angle))*scale},
            .color={218/255.f,178/255.f,69/255.f,0}};
        if(i<32){fan[i*3]=0;fan[i*3+1]=i+1;fan[i*3+2]=i+2;}
    }
    if(ok)ok=SDL_RenderGeometry(r,NULL,glow,34,fan,96) &&
        b2_box(r,box,scale,(SDL_Color){138,113,53,255},true) &&
        b2_box(r,(SvLoginRect){box.x+2,box.y+2,box.w-4,box.h-4},scale,(SDL_Color){2,5,3,204},true) &&
        b2_box(r,(SvLoginRect){box.x+8,box.y+8,box.w-16,box.h-16},scale,(SDL_Color){146,121,56,51},true) &&
        b2_box(r,(SvLoginRect){box.x+1,box.y+1,box.w-2,1},scale,(SDL_Color){255,230,137,31},false);
    SDL_Vertex star[9];int star_indices[24];
    float sx=box.x+box.w-20,sy=box.y+20;
    star[0]=(SDL_Vertex){.position={sx*scale,sy*scale},.color={211/255.f,174/255.f,65/255.f,.28f}};
    for(int i=0;i<8;++i) {
        float angle=i*SDL_PI_F/4,extent=i%2?1.4f:5;
        star[i+1]=(SDL_Vertex){.position={(sx+extent*SDL_cosf(angle))*scale,(sy+extent*SDL_sinf(angle))*scale},.color=star[0].color};
        star_indices[i*3]=0;star_indices[i*3+1]=i+1;star_indices[i*3+2]=i==7?1:i+2;
    }
    if(ok)ok=SDL_RenderGeometry(r,NULL,star,9,star_indices,24);
    SDL_Rect inner={(int)SDL_ceilf((box.x+9)*scale),(int)SDL_ceilf((box.y+9)*scale),
        (int)SDL_floorf((box.w-18)*scale),(int)SDL_floorf((box.h-18)*scale)};
    if(!SDL_GetRectIntersection(&clip,&inner,&inner))return SDL_SetRenderClipRect(r,clipped?&previous:NULL) && ok;
    if(!SDL_SetRenderClipRect(r,&inner))return false;
    float pad=short_height?19:23,top=short_height?13:23;
    SvLoginRect content={box.x+pad,box.y+top,box.w-2*pad,35};
    float row_heights[6],rows_height=0;
    unsigned columns=(unsigned)((content.w-64)/(15*.61f));if(!columns)columns=1;
    for(unsigned i=0;i<6;++i) {
        unsigned lines=values && values[i]?b2_wrap_count(values[i],columns):1;
        row_heights[i]=lines>1?SDL_max(short_height?22:33,lines*22.5f+4):(short_height?22:33);rows_height+=row_heights[i];
    }
    float stats_y=content.y+(name && *name?35:0)+(short_height?8:14)+1+(short_height?8:20)+rows_height+
        (short_height?6:10)+15+(short_height?3:8);
    float stat_height=short_height?SDL_max(28,SDL_min(34,(box.y+box.h-12-stats_y-10)/6)):42;
    if(viewport) {
        float end=stats_y+6*stat_height+5*(short_height?2:4)+12;
        viewport->summary_max=end>box.y+box.h?(unsigned)SDL_ceilf(end-box.y-box.h):0;
        if(viewport->summary_scroll>viewport->summary_max)viewport->summary_scroll=viewport->summary_max;
        content.y-=viewport->summary_scroll;
    }
    if(ok && name && *name)ok=b2_character_text(r,name,content,30,0,scale,(SDL_Color){241,212,108,255},true,false,false);
    float y=content.y+(name && *name?35:0)+(short_height?8:14);
    if(ok)ok=b2_box(r,(SvLoginRect){content.x,y,content.w,1},scale,(SDL_Color){141,118,53,107},false);
    y+=1+(short_height?8:20);
    static const char *labels[]={"SEX","CLASS","RACE","TRAIT","BODY","MODE"};
    for(unsigned i=0;ok && i<6;++i) {
        ok=b2_character_text(r,labels[i],(SvLoginRect){content.x,y+(short_height?2:0),content.w,19},15,.9f,scale,
                (SDL_Color){133,138,128,255},false,false,false) &&
            b2_box(r,(SvLoginRect){content.x,y+row_heights[i]-(short_height?1:9),content.w,1},scale,(SDL_Color){88,82,54,87},false);
        if(ok && values && values[i] && strcmp(values[i],"--"))
            ok=b2_creation_value(r,values[i],(SvLoginRect){content.x+64,y+(short_height?2:0),content.w-64,19},scale);
        y+=row_heights[i];
    }
    y-=short_height?0:8;
    y+=short_height?6:18;
    if(ok)ok=b2_character_text(r,"ATTRIBUTES",(SvLoginRect){content.x,y,content.w,18},13.5f,1.62f,scale,
        (SDL_Color){166,145,80,255},false,false,false);
    y+=15+(short_height?3:8);
    static const char *stats[]={"STRENGTH","INTELLIGENCE","WISDOM","DEXTERITY","CONSTITUTION","CHARISMA"};
    for(unsigned i=0;ok && i<6;++i) {
        float height=stat_height;
        SvLoginRect stat={content.x,y,content.w,height};
        ok=b2_box(r,stat,scale,(SDL_Color){4,8,5,158},false) &&
            b2_box(r,stat,scale,(SDL_Color){91,84,52,117},true) &&
            b2_character_text(r,stats[i],(SvLoginRect){stat.x+9,stat.y+(height-15)/2,stat.w-115,18},13.5f,.81f,scale,
                (SDL_Color){127,132,123,255},false,false,false) &&
            b2_character_text(r,numbers?numbers[i]:"10",(SvLoginRect){stat.x+stat.w-109,stat.y+(height-30)/2,99,32},25.5f,0,scale,
                (SDL_Color){232,199,83,255},true,false,true);
        y+=height+(short_height?2:4);
    }
    return SDL_SetRenderClipRect(r,clipped?&previous:NULL) && ok;
}

static bool b2_characters(SDL_Renderer *r,SvFont *font,const SvEndpoint *endpoint,const SvEndpointInput *input)
{
    if(!sv_login_shell_render(r,endpoint,input))return false;
    int w,h;if(!SDL_GetCurrentRenderOutputSize(r,&w,&h))return false;
    float scale=SDL_GetWindowDisplayScale(SDL_GetRenderWindow(r));if(scale<=0)scale=1;
    const SvLoginView *v=input->login_view;
    SvLoginLayout l=sv_character_shell_layout(w/scale,h/scale,v->ordinary_name_entry);
    SvCharacterLayout c=sv_character_content_layout(sv_character_layout(l,v->first),v->count);
    if(v->ordinary_name_entry) {
        const SvTextField *field=v->name_field;
        if(!field)return false;
        bool short_height=h/scale<=800;
        const char *title="Create a Character",*subtitle="Shape a new adventurer for the selected world.";
        float title_width,intro_width;
        if(!b2_box(r,(SvLoginRect){l.panel.x+1,l.panel.y+1,l.panel.w-2,50},scale,(SDL_Color){21,24,17,174},false) ||
           !sv_login_shell_heading(r,title,l.panel.x+20,l.panel.y+7,19,scale,(SDL_Color){239,208,105,255}) ||
           !b2_text(r,subtitle,l.panel.x+20,l.panel.y+32,9,scale,b2_muted) ||
           !sv_login_shell_measure(r,title,19,scale,true,&title_width) ||
           !sv_login_shell_measure(r,subtitle,9,scale,false,&intro_width) ||
           !b2_box(r,(SvLoginRect){l.panel.x,l.panel.y+51,l.panel.w,1},scale,(SDL_Color){48,42,27,255},false) ||
           !b2_box(r,c.footer,scale,(SDL_Color){4,7,5,184},false) ||
           !b2_box(r,(SvLoginRect){c.footer.x,c.footer.y,c.footer.w,1},scale,(SDL_Color){48,42,27,255},false))return false;
        float rule_x=l.panel.x+20+title_width+8,rule_width=SDL_max(0,intro_width-title_width-8);
        SDL_Vertex rule[]={
            {.position={rule_x*scale,(l.panel.y+18)*scale},.color={118/255.f,95/255.f,52/255.f,1}},
            {.position={(rule_x+rule_width)*scale,(l.panel.y+18)*scale},.color={118/255.f,95/255.f,52/255.f,0}},
            {.position={(rule_x+rule_width)*scale,(l.panel.y+19)*scale},.color={118/255.f,95/255.f,52/255.f,0}},
            {.position={rule_x*scale,(l.panel.y+19)*scale},.color={118/255.f,95/255.f,52/255.f,1}}};
        const int indices[]={0,1,2,0,2,3};
        float stage_top=l.panel.y+(short_height?70:74),stage_width=l.panel.x+l.panel.w-23-c.name.x;
        if(!SDL_RenderGeometry(r,NULL,rule,4,indices,6) ||
           !b2_creation_summary(r,field->bytes,NULL,NULL,NULL,c.summary,scale,short_height) ||
           !sv_login_shell_heading(r,"Choose a name",c.name.x,stage_top,short_height?20:22,scale,(SDL_Color){238,231,212,255}) ||
           !b2_name_intro(r,(SvLoginRect){c.name.x,stage_top+(short_height?27:33),stage_width,0},scale,short_height) ||
           !b2_text(r,"Character name",c.name.x,c.name.y-19,11,scale,(SDL_Color){168,169,159,255}) ||
           !b2_account_field(r,font,c.name,field->bytes,field->cursor,field->anchor,scale,!v->pending,false) ||
           !b2_character_action(r,c.random,"* RANDOM",scale,false,!v->pending,false) ||
           !b2_character_action(r,c.back,"BACK",scale,false,!v->pending,false) ||
           !b2_character_action(r,c.enter,"CONTINUE",scale,false,!v->pending,true) ||
           !b2_wrap(r,"1–15 characters. Letters, numbers, spaces and . , - ' & _ $ % ~ # are allowed. The first letter is capitalised.",
                (SvLoginRect){c.name.x,c.name.y+93,c.name.w+c.random.w+8,0},13.5f,scale,(SDL_Color){116,122,114,255}) ||
           !b2_character_text(r,v->pending?"Waiting for name acceptance...":"Enter confirm · blank = random · Esc cancel",
                (SvLoginRect){c.back.x+c.back.w+20,c.hint.y,c.enter.x-c.back.x-c.back.w-30,c.hint.h},12,0,scale,(SDL_Color){94,98,92,255},false,false,true))return false;
        const char *message=input->text_error==SV_TEXT_ENCODING_ERROR?"Cannot encode that character as a server field byte.":
            input->clipboard_unavailable?"Clipboard unavailable; name draft unchanged.":
            input->text_error==SV_TEXT_TRUNCATED?"Name is limited to 15 server bytes.":NULL;
        if(message && !b2_status(r,message,(SvLoginRect){c.name.x,c.name.y+47,c.name.w+c.random.w+8,40},scale,true))return false;
        if(input->contact_status && strcmp(input->contact_status,"New character name: Escape cancels and returns to the overview."))
            return b2_status(r,input->contact_status,(SvLoginRect){c.name.x,c.name.y+149,stage_width,90},scale,false);
        return true;
    }
    size_t total=v->count+v->free_slots[0]+v->free_slots[1]+v->free_slots[2];
    char subtitle[120];SDL_snprintf(subtitle,sizeof(subtitle),"(You can create up to %zu different characters to play with)",total);
    float title_width;
    if(!b2_box(r,(SvLoginRect){l.panel.x+1,l.panel.y+1,l.panel.w-2,50},scale,(SDL_Color){21,24,17,174},false) ||
       !sv_login_shell_heading(r,"Character Overview",l.panel.x+20,l.panel.y+7,19,scale,(SDL_Color){239,208,105,255}) ||
       !sv_login_shell_measure(r,"Character Overview",19,scale,true,&title_width) ||
       !b2_text(r,subtitle,l.panel.x+20,l.panel.y+32,9,scale,(SDL_Color){119,125,117,255}) ||
       !b2_box(r,(SvLoginRect){l.panel.x,l.panel.y+51,l.panel.w,1},scale,(SDL_Color){48,42,27,255},false) ||
       !b2_box(r,c.footer,scale,(SDL_Color){4,7,5,184},false) ||
       !b2_box(r,(SvLoginRect){c.footer.x,c.footer.y,c.footer.w,1},scale,(SDL_Color){48,42,27,255},false))return false;
    float rule_x=l.panel.x+20+title_width+8;
    SDL_Vertex ornament[]={
        {.position={rule_x*scale,(l.panel.y+18)*scale},.color={118/255.f,95/255.f,52/255.f,1}},
        {.position={(rule_x+160)*scale,(l.panel.y+18)*scale},.color={118/255.f,95/255.f,52/255.f,0}},
        {.position={(rule_x+160)*scale,(l.panel.y+19)*scale},.color={118/255.f,95/255.f,52/255.f,0}},
        {.position={rule_x*scale,(l.panel.y+19)*scale},.color={118/255.f,95/255.f,52/255.f,1}}};
    const int indices[]={0,1,2,0,2,3};
    if(!SDL_RenderGeometry(r,NULL,ornament,4,indices,6) ||
       !b2_character_text(r,"CHOOSE AN EXISTING CHARACTER:",(SvLoginRect){c.list.x,l.panel.y+70,c.list.w,14},10,1.1f,scale,
            (SDL_Color){169,159,124,255},false,false,false))return false;
    /* Reordering is a later capability. These hints describe actual bindings. */
    const char *creation_hint="Create a new slot-exclusive character (IDDC or PvP only)";
    float hint_width,quit_width;
    if(!sv_login_shell_measure(r,creation_hint,11,scale,false,&hint_width) ||
       !sv_login_shell_measure(r,"Quit the game",11,scale,false,&quit_width))return false;
    float quit_group=31+quit_width;
    bool wrap_hint=31+hint_width+16+quit_group>c.list.w;
    float q_x=wrap_hint?c.list.x+c.list.w-quit_group:c.list.x+31+hint_width+16;
    if(!b2_round(r,(SvLoginRect){c.list.x,l.panel.y+90,23,21},3,scale,(SDL_Color){98,87,47,255},true) ||
       !b2_center(r,"E",(SvLoginRect){c.list.x,l.panel.y+90,23,21},l.panel.y+94,11,scale,(SDL_Color){225,192,88,255}) ||
       !(wrap_hint?
            b2_text(r,"Create a new slot-exclusive character",c.list.x+31,l.panel.y+88,11,scale,(SDL_Color){155,158,149,255}) &&
            b2_text(r,"(IDDC or PvP only)",c.list.x+31,l.panel.y+102,11,scale,(SDL_Color){155,158,149,255}):
            b2_text(r,creation_hint,c.list.x+31,l.panel.y+94,11,scale,(SDL_Color){155,158,149,255})) ||
       !b2_round(r,(SvLoginRect){q_x,l.panel.y+90,23,21},3,scale,(SDL_Color){98,87,47,255},true) ||
       !b2_center(r,"Q",(SvLoginRect){q_x,l.panel.y+90,23,21},l.panel.y+94,11,scale,(SDL_Color){225,192,88,255}) ||
       !b2_text(r,"Quit the game",q_x+31,l.panel.y+94,11,scale,(SDL_Color){155,158,149,255}) ||
       !b2_box(r,(SvLoginRect){c.list.x,c.list.y-8,c.list.w-4,1},scale,(SDL_Color){48,42,27,255},false))return false;
    /* Level diamonds extend 1.27 logical units past their 28-unit row. Keep
     * their tips and AA fringe visible without changing input geometry. */
    int vertical_fringe=(int)SDL_ceilf(1.3f*scale+1);
    SDL_Rect clip={(int)SDL_floorf(c.list.x*scale)-1,(int)SDL_floorf(c.list.y*scale)-vertical_fringe,0,0};
    clip.w=(int)SDL_ceilf((c.list.x+c.list.w)*scale)-clip.x+1;
    clip.h=(int)SDL_ceilf((c.list.y+c.list.h)*scale)-clip.y+vertical_fringe;
    if(!SDL_SetRenderClipRect(r,&clip))return false;
    bool ok=true;
    for(size_t i=c.first;ok && i<total && i<c.first+c.rows;++i) {
        SvLoginRect row=sv_character_row(c,i);
        if(i<v->count) {
            const SvLoginRowView *character=&v->rows[i];
            char letter[4],level[16],build[160],mode[32];
            SDL_snprintf(letter,sizeof(letter),"%c)",(int)('a'+i));
            SDL_snprintf(level,sizeof(level),"%d",character->level);
            SDL_snprintf(build,sizeof(build),"%s %s",character->race,character->class_title);
            b2_character_mode(mode,character->mode);
            float name_x=row.x+68,mode_x=row.x+row.w-240,identity_width=mode_x-name_x-8;
            float name_width=(identity_width-10)*1.15f/2.15f,build_x=name_x+name_width+10;
            ok=b2_character_row_frame(r,row,scale,i==v->selected,b2_hover(r,row,scale)) &&
               b2_character_text(r,letter,(SvLoginRect){row.x+12,row.y+9,16,16},11,0,scale,(SDL_Color){223,189,80,255},false,true,false) &&
               b2_level_badge(r,row,scale) &&
               b2_character_text(r,level,(SvLoginRect){row.x+36,row.y+9,24,16},10,0,scale,(SDL_Color){243,200,76,255},true,true,false) &&
               b2_character_text(r,character->name,(SvLoginRect){name_x,row.y+4,name_width,22},17,0,scale,b2_normal,true,false,false) &&
               b2_character_text(r,build,(SvLoginRect){build_x,row.y+9,mode_x-build_x-8,16},11,0,scale,(SDL_Color){163,166,158,255},false,false,false) &&
               b2_box(r,(SvLoginRect){mode_x,row.y+2,98,24},scale,(SDL_Color){69,73,63,255},true) &&
               b2_character_text(r,mode,(SvLoginRect){mode_x+4,row.y+9,90,16},9,.63f,scale,(SDL_Color){187,183,168,255},false,true,false) &&
               b2_character_text(r,character->location[0]?character->location:"Unavailable",
                   (SvLoginRect){mode_x+106,row.y+9,122,16},10,0,scale,(SDL_Color){119,125,117,255},false,false,true);
        } else {
            size_t placeholder=i-v->count;
            bool dedicated=placeholder>=v->free_slots[0];
            const char *label=!dedicated?"<free slot>":placeholder<v->free_slots[0]+v->free_slots[1]?"<free IDDC-exclusive slot>":"<free PvP-exclusive slot>";
            ok=b2_character_placeholder(r,row,label,scale,dedicated);
        }
    }
    if(!total)ok=b2_text(r,"No characters or free slots available.",c.list.x,c.list.y+10,13,scale,b2_muted);
    if(!SDL_SetRenderClipRect(r,NULL) || !ok)return false;
    if(total>c.rows) {
        float thumb=c.list.h*c.rows/total;
        if(!b2_box(r,(SvLoginRect){c.list.x+c.list.w-3,c.list.y+(c.list.h-thumb)*c.first/(total-c.rows),3,thumb},scale,(SDL_Color){92,81,46,255},false))return false;
    }
    char status[100],hint[80];
    SDL_snprintf(status,sizeof(status),"%zu existing character%s",v->count,v->count==1?"":"s");
    bool ordinary_hint=v->focus==1 || b2_hover(r,c.ordinary,scale);
    bool exclusive_hint=v->focus==2 || b2_hover(r,c.exclusive,scale);
    if((ordinary_hint || exclusive_hint) && !v->creation_supported)
        SDL_strlcpy(status,"Character creation unavailable",sizeof(status));
    else if(exclusive_hint && !v->exclusive_creation_available)
        SDL_strlcpy(status,v->first_run?"First login: exclusive disabled":"No exclusive slots available",sizeof(status));
    else if(ordinary_hint && !v->ordinary_creation_available)
        SDL_strlcpy(status,"No ordinary slots available",sizeof(status));
    if(v->count)SDL_snprintf(hint,sizeof(hint),"Up/Down Enter a-%c",(int)('a'+v->count-1));
    else hint[0]=0;
    return b2_character_action(r,c.quit,"BACK",scale,v->focus==4,true,false) &&
        b2_character_action(r,c.ordinary,"N CREATE CHARACTER",scale,v->focus==1,v->ordinary_creation_available && v->creation_supported,false) &&
        b2_character_action(r,c.exclusive,"E EXCLUSIVE CHARACTER",scale,v->focus==2,v->exclusive_creation_available && v->creation_supported,false) &&
        b2_character_text(r,status,c.status,11,0,scale,b2_muted,false,true,false) &&
        (c.hint.w<=0 || b2_character_text(r,hint,c.hint,9,0,scale,(SDL_Color){94,98,92,255},false,true,false)) &&
        b2_character_action(r,c.enter,v->pending?"WAIT...":"PLAY",scale,v->focus==3,v->count && !v->pending,true);
}

static bool b2_clip(SDL_Renderer *r,SvLoginRect box,float scale)
{
    SDL_Rect clip={(int)(box.x*scale),(int)(box.y*scale),(int)(box.w*scale),(int)(box.h*scale)};
    return SDL_SetRenderClipRect(r,&clip);
}
/* Word wrap is shared by measurement and drawing, preserving long server
 * titles and descriptions without reducing their font size. */
static unsigned b2_wrap_count(const char *text,unsigned columns)
{
    unsigned lines=0;size_t at=0;
    if(!columns)columns=1;
    while(text[at]) {
        size_t count=strlen(text+at);if(count>columns)count=columns;
        if(text[at+count]) {size_t split=count;while(split && text[at+split]!=' ')--split;if(split)count=split;}
        at+=count;while(text[at]==' ')++at;++lines;
    }
    return lines;
}
static bool b2_wrap_leading(SDL_Renderer *r,const char *text,SvLoginRect box,float size,float scale,SDL_Color color,float leading)
{
    unsigned columns=(unsigned)(box.w/(size*.61f));if(!columns)columns=1;
    size_t at=0;float y=box.y;
    while(text[at]) {
        size_t count=strlen(text+at);if(count>columns)count=columns;
        if(text[at+count]) {size_t split=count;while(split && text[at+split]!=' ')--split;if(split)count=split;}
        char line[256];if(count>=sizeof(line))count=sizeof(line)-1;
        memcpy(line,text+at,count);line[count]=0;
        if(!b2_text(r,line,box.x,y,size,scale,color))return false;
        at+=count;while(text[at]==' ')++at;y+=size*leading;
    }
    return true;
}
static bool b2_wrap(SDL_Renderer *r,const char *text,SvLoginRect box,float size,float scale,SDL_Color color)
{ return b2_wrap_leading(r,text,box,size,scale,color,1.5f); }
/* Short card copy from frozen reference login.html creationData; native
 * IDs and legal lists are the only source of availability and ordering. */
static const char *b2_birth_card_copy(const SvBirthView *v,int id)
{
    if(v->owner==SV_BIRTH_SEX)return id==SV_BIRTH_SEX_MALE?"Male character.":"Female character.";
    if(v->owner==SV_BIRTH_BODY)return id==SV_BIRTH_BODY_NORMAL?
        sv_birth_normal_copy:sv_birth_bat_copy;
    if(v->owner!=SV_BIRTH_MODE)return "";
    switch((SvBirthMode)id) {
    case SV_BIRTH_MODE_NORMAL:return "Three lives and the standard TomeNET progression.";
    case SV_BIRTH_MODE_NO_GHOST:return "One life only — the traditional roguelike way.";
    case SV_BIRTH_MODE_SOLOIST:return "One life and no trading, parties or guilds.";
    case SV_BIRTH_MODE_EVERLASTING:return "Unlimited resurrection, but no high-score entry.";
    case SV_BIRTH_MODE_HELLISH:return "One life with additional difficulty.";
    case SV_BIRTH_MODE_IDDC:return "Ironman Deep Dive Challenge — one life in an isolated dungeon.";
    case SV_BIRTH_MODE_IDDC_SOLOIST:return "IDDC without trading with other players.";
    case SV_BIRTH_MODE_IDDC_HELLISH:return "An extra-hard Ironman Deep Dive Challenge.";
    case SV_BIRTH_MODE_PVP:case SV_BIRTH_MODE_DEDICATED_PVP:return "Special player-versus-player rules replace normal victory.";
    case SV_BIRTH_MODE_FRUIT_BAT:return "Use the fruit bat body in this server's historical mode selection.";
    default:return "";
    }
}
static char b2_birth_hotkey(const SvBirthView *v,unsigned p)
{
    int id=sv_birth_choice_id(v,p);
    if(v->owner==SV_BIRTH_SEX)return id==SV_BIRTH_SEX_MALE?'m':'f';
    if(v->owner==SV_BIRTH_BODY)return id==SV_BIRTH_BODY_NORMAL?'n':'f';
    if(v->owner==SV_BIRTH_MODE) {
        switch((SvBirthMode)id) {
        case SV_BIRTH_MODE_NORMAL:return 'n';case SV_BIRTH_MODE_NO_GHOST:return 'g';
        case SV_BIRTH_MODE_SOLOIST:case SV_BIRTH_MODE_IDDC_SOLOIST:return 's';
        case SV_BIRTH_MODE_HELLISH:case SV_BIRTH_MODE_IDDC_HELLISH:return 'H';
        case SV_BIRTH_MODE_EVERLASTING:return 'e';case SV_BIRTH_MODE_FRUIT_BAT:return 'f';
        case SV_BIRTH_MODE_PVP:case SV_BIRTH_MODE_DEDICATED_PVP:return 'p';
        case SV_BIRTH_MODE_IDDC:return 'i';default:return 0;
        }
    }
    unsigned index=v->owner==SV_BIRTH_RACE || v->owner==SV_BIRTH_STATS?(unsigned)id:p;
    return index<26?(char)('a'+index):0;
}
static void b2_stat_format(char *text,size_t capacity,int value)
{
    if(value>18)snprintf(text,capacity,"18/%02d",(value-18)*10);
    else snprintf(text,capacity,"%d",value);
}
static const char *b2_birth_description(SvBirthOwner owner,const char *title)
{
    for(size_t i=0;i<sizeof(birth_descriptions)/sizeof(birth_descriptions[0]);++i)
        if(birth_descriptions[i].owner==owner && !strcmp(birth_descriptions[i].title,title))return birth_descriptions[i].copy;
    if(owner==SV_BIRTH_BODY)return !strcmp(title,"Fruit bat")?
        "Use the fruit bat body. Body choice and its restrictions are supplied by the native creation interaction.":
        "Use the normal body for your selected race.";
    return "No description is available for this server-defined choice.";
}
static const char *b2_setup_title(const SvBirthView *v,int id,SvBirthOwner owner)
{
    if(id<0 || !v->setup)return "--";
    if(owner==SV_BIRTH_CLASS && id<v->setup->class_count)return v->setup->classes[id].title;
    if(owner==SV_BIRTH_RACE && id<v->setup->race_count)return v->setup->races[id].title;
    if(owner==SV_BIRTH_TRAIT && id<v->setup->trait_count)return v->setup->traits[id].title;
    return "--";
}
static bool b2_birth_summary(SDL_Renderer *r,const SvEndpointInput *input,SvBirthLayout l,float scale)
{
    const SvBirthView *v=input->birth_view;
    SvBirthViewport *viewport=input->birth_viewport;
    const char *name=input->login_view->character_name_draft;
    const char *values[]={v->chosen_sex==SV_BIRTH_SEX_NONE?"--":v->chosen_sex==SV_BIRTH_SEX_MALE?"Male":"Female",
        b2_setup_title(v,v->class_confirmed?v->chosen_class:-1,SV_BIRTH_CLASS),
        b2_setup_title(v,v->race_confirmed?v->chosen_race:-1,SV_BIRTH_RACE),
        b2_setup_title(v,v->trait_confirmed?v->chosen_trait:-1,SV_BIRTH_TRAIT),
        v->body_confirmed?(v->chosen_body==SV_BIRTH_BODY_FRUIT_BAT?"Fruit bat":"Normal body"):"--",
        v->mode_confirmed?sv_birth_mode_title(v->chosen_mode):"--"};
    for(unsigned p=0;p<sv_birth_choice_count(v);++p) {
        if(sv_birth_choice_id(v,p)!=sv_birth_highlight(v))continue;
        unsigned index=v->owner==SV_BIRTH_SEX?0:v->owner==SV_BIRTH_CLASS?1:
            v->owner==SV_BIRTH_RACE?2:v->owner==SV_BIRTH_TRAIT?3:
            v->owner==SV_BIRTH_BODY?4:v->owner==SV_BIRTH_MODE?5:6;
        if(index<6)values[index]=sv_birth_choice_title(v,p);
    }
    char formatted[6][24];const char *numbers[6];
    for(unsigned i=0;i<6;++i) {
        /* Preview the current selection without changing the birth allocation. */
        int effective=v->stats_point_buy && v->stats_prepared?v->stats[i]:10;
        int cls=v->owner==SV_BIRTH_CLASS?v->highlighted_class:v->class_confirmed?v->chosen_class:-1;
        int race=v->owner==SV_BIRTH_RACE?v->highlighted_race:v->race_confirmed?v->chosen_race:-1;
        if(v->setup && cls>=0 && cls<v->setup->class_count)effective+=v->setup->classes[cls].adjustment[i];
        if(v->setup && race>=0 && race<v->setup->race_count)effective+=v->setup->races[race].adjustment[i];
        b2_stat_format(formatted[i],sizeof(formatted[i]),effective);numbers[i]=formatted[i];
    }
    return b2_creation_summary(r,name,values,numbers,viewport,l.summary,scale,l.summary.h<600);
}
static bool b2_stat_button(SDL_Renderer *r,SvLoginRect box,const char *label,float scale,bool enabled)
{
    SvLoginRect text=box;text.y+=(box.h-18)/2;text.h=18;
    return b2_box(r,box,scale,(SDL_Color){8,11,8,255},false) &&
        b2_box(r,box,scale,enabled?(SDL_Color){77,73,53,255}:(SDL_Color){46,49,43,255},true) &&
        b2_character_text(r,label,text,15,0,scale,enabled?(SDL_Color){212,183,88,255}:(SDL_Color){79,83,76,255},false,true,true);
}
static bool b2_birth_stat_selection(SDL_Renderer *r,SvLoginRect row,float scale,
                                    bool chosen,const SvBirthViewport *viewport)
{
    bool ok=(!chosen || b2_box(r,row,scale,(SDL_Color){91,70,19,71},false)) &&
        b2_box(r,(SvLoginRect){row.x,row.y+row.h-1,row.w,1},scale,(SDL_Color){36,40,32,255},false);
    if(ok && chosen)ok=b2_box(r,(SvLoginRect){row.x,row.y,3,row.h},scale,(SDL_Color){209,173,63,255},false);
    /* CSS :focus-visible is separate from the persistent selected-row fill. */
    if(ok && chosen && viewport->choice_keyboard_focus && viewport->focus==0)
        ok=b2_box(r,(SvLoginRect){row.x,row.y,row.w-1,row.h-1},scale,(SDL_Color){215,181,72,255},true);
    return ok;
}
static bool b2_birth_stat_row(SDL_Renderer *r,const SvBirthView *v,unsigned id,
                               SvLoginRect row,float scale)
{
    static const char *names[]={"Strength","Intelligence","Wisdom","Dexterity","Constitution","Charisma"};
    const SvContactClass *cls=&v->setup->classes[v->chosen_class];
    const SvContactRace *race=&v->setup->races[v->chosen_race];
    int effective=v->stats[id]+cls->adjustment[id]+race->adjustment[id];
    bool chosen=(int)id==v->highlighted_stat;
    char base[16],cm[16],rm[16],current[24],recommend[32],minimum[24];
    snprintf(base,sizeof(base),"%d",v->stats[id]);
    snprintf(cm,sizeof(cm),cls->adjustment[id]>0?"+%d":"%d",cls->adjustment[id]);
    snprintf(rm,sizeof(rm),race->adjustment[id]>0?"+%d":"%d",race->adjustment[id]);
    b2_stat_format(current,sizeof(current),effective);
    unsigned rec=cls->recommendation[id],min=rec>=100?rec-100:rec;
    b2_stat_format(minimum,sizeof(minimum),(int)min);
    snprintf(recommend,sizeof(recommend),"%s%s",min?minimum:"",rec>=100?" (main)":"");
    SDL_Color value=chosen?(SDL_Color){255,141,0,255}:(SDL_Color){199,157,85,255};
    /* Limit colors describe the point-buy allocation, before class/race mods. */
    if(v->stats[id]==8)value=chosen?(SDL_Color){255,48,48,255}:(SDL_Color){183,0,0,255};
    if(v->stats[id]==17)value=chosen?(SDL_Color){0,255,255,255}:(SDL_Color){175,0,255,255};
    SvLoginRect cells[8];for(unsigned i=0;i<8;++i)cells[i]=sv_birth_stat_cell(row,i);
    bool ok=b2_box(r,(SvLoginRect){cells[4].x+cells[4].w/2,row.y+7,1,row.h-14},scale,(SDL_Color){112,101,59,117},false);
    const char *texts[]={names[id],base,cm,rm,"",current,"",recommend};
    float sizes[]={row.w<800?14:18,21,15,15,0,21,0,13};
    SDL_Color colors[]={{215,211,196,255},{184,182,170,255},
        cls->adjustment[id]<0?(SDL_Color){217,121,100,255}:(SDL_Color){210,202,168,255},
        race->adjustment[id]<0?(SDL_Color){217,121,100,255}:(SDL_Color){210,202,168,255},
        {0},value,{0},{85,189,114,255}};
    for(unsigned i=0;ok && i<8;++i)if(i!=4 && i!=6) {
        SvLoginRect text=cells[i];text.y+=(row.h-sizes[i]*1.2f)/2;text.h=sizes[i]*1.3f;
        ok=b2_character_text(r,texts[i],text,sizes[i],0,scale,colors[i],false,i!=0,false);
    }
    return ok && b2_stat_button(r,sv_birth_stat_minus(row),"-",scale,v->stat_can_decrease[id]) &&
        b2_stat_button(r,sv_birth_stat_plus(row),"+",scale,v->stat_can_increase[id]);
}
static bool b2_birth_choices_clip(SDL_Renderer *r,SvBirthLayout layout,SvBirthOwner owner,float scale)
{
    if(owner!=SV_BIRTH_CLASS && owner!=SV_BIRTH_RACE && owner!=SV_BIRTH_TRAIT)return b2_clip(r,layout.choices,scale);
    SvLoginRect box=layout.choices;
    int x=(int)SDL_floorf(box.x*scale)-1,y=(int)SDL_floorf(box.y*scale)-1;
    SDL_Rect clip={x,y,(int)SDL_ceilf((box.x+box.w)*scale)+2-x,(int)SDL_ceilf((box.y+box.h)*scale)+2-y};
    return SDL_SetRenderClipRect(r,&clip);
}
static bool b2_birth_detail_title(SDL_Renderer *r,const char *text,SvLoginRect box,float scale)
{
    unsigned columns=(unsigned)(box.w/(22*.61f));if(!columns)columns=1;
    size_t at=0,length=strlen(text);
    while(at<length) {
        size_t count=length-at;if(count>columns)count=columns;
        if(at+count<length){size_t split=count;while(split && text[at+split]!=' ')--split;if(split)count=split;}
        char line[SV_CONTACT_NAME_CAPACITY];if(count>=sizeof(line))count=sizeof(line)-1;
        memcpy(line,text+at,count);line[count]=0;
        if(!sv_login_shell_heading(r,line,box.x,box.y,22,scale,b2_gold))return false;
        at+=count;while(text[at]==' ')++at;box.y+=33;
    }
    return true;
}
static bool b2_class_guidance(SDL_Renderer *r,SvBirthLayout l,float scale)
{
    SvLoginRect box={l.heading.x,l.panel.y+112,l.heading.w,sv_class_guidance_height(l.heading.w)};
    if(!b2_box(r,box,scale,(SDL_Color){4,8,5,158},false) || !b2_box(r,box,scale,(SDL_Color){91,84,52,117},true))return false;
    const SDL_Color colors[]={{85,189,114,255},{220,159,62,255},{217,88,78,255}};
    unsigned columns=(unsigned)((box.w-26)/(15*.61f));if(!columns)columns=1;
    float y=box.y+11;
    for(unsigned i=0;i<3;++i) {
        char text[160];SDL_snprintf(text,sizeof(text),"%s%s",sv_class_guidance_prefix[i],sv_class_guidance_suffix[i]);
        size_t at=0,length=strlen(text),prefix_length=strlen(sv_class_guidance_prefix[i]);
        while(at<length) {
            size_t count=length-at;if(count>columns)count=columns;
            if(at+count<length){size_t split=count;while(split && text[at+split]!=' ')--split;if(split)count=split;}
            size_t colored=at<prefix_length?SDL_min(count,prefix_length-at):0;
            char line[160];memcpy(line,text+at,colored);line[colored]=0;
            float width=0;
            if(colored && (!b2_text(r,line,box.x+13,y,15,scale,colors[i]) || !sv_login_shell_measure(r,line,15,scale,false,&width)))return false;
            memcpy(line,text+at+colored,count-colored);line[count-colored]=0;
            if(*line && !b2_text(r,line,box.x+13+width,y,15,scale,(SDL_Color){185,187,178,255}))return false;
            at+=count;while(text[at]==' ')++at;y+=22.5f;
        }
        y+=5;
    }
    return true;
}
static bool b2_birth(SDL_Renderer *r,const SvEndpoint *endpoint,const SvEndpointInput *input)
{
    if(!sv_login_shell_render(r,endpoint,input))return false;
    int w,h;if(!SDL_GetCurrentRenderOutputSize(r,&w,&h))return false;
    float scale=SDL_GetWindowDisplayScale(SDL_GetRenderWindow(r));if(scale<=0)scale=1;
    const SvBirthView *v=input->birth_view;SvBirthViewport *viewport=input->birth_viewport;
    if(!v || !viewport)return false;
    SvBirthLayout l=sv_birth_layout(w/scale,h/scale,v->owner);
    static const char *titles[]={"","Choose sex","Choose a class","Choose a race","Choose a trait","Choose a body","Distribute attributes","Choose a game mode","Creation submitted"};
    bool waiting=v->owner==SV_BIRTH_READY;
    SvLoginRect footer=l.footer;
        const char *title="Create a Character",*subtitle="Shape a new adventurer for the selected world.";
        float title_width,intro_width;
        if(!b2_box(r,(SvLoginRect){l.panel.x+1,l.panel.y+1,l.panel.w-2,50},scale,(SDL_Color){21,24,17,174},false) ||
           !sv_login_shell_heading(r,title,l.panel.x+20,l.panel.y+7,19,scale,(SDL_Color){239,208,105,255}) ||
           !b2_text(r,subtitle,l.panel.x+20,l.panel.y+32,9,scale,b2_muted) ||
           !sv_login_shell_measure(r,title,19,scale,true,&title_width) ||
           !sv_login_shell_measure(r,subtitle,9,scale,false,&intro_width) ||
           !b2_box(r,(SvLoginRect){l.panel.x,l.panel.y+51,l.panel.w,1},scale,(SDL_Color){48,42,27,255},false) ||
           !b2_box(r,footer,scale,(SDL_Color){4,7,5,184},false) ||
           !b2_box(r,(SvLoginRect){footer.x,footer.y,footer.w,1},scale,(SDL_Color){48,42,27,255},false))return false;
        float rule_x=l.panel.x+20+title_width+8,rule_width=SDL_max(0,intro_width-title_width-8);
        SDL_Vertex rule[]={
            {.position={rule_x*scale,(l.panel.y+18)*scale},.color={118/255.f,95/255.f,52/255.f,1}},
            {.position={(rule_x+rule_width)*scale,(l.panel.y+18)*scale},.color={118/255.f,95/255.f,52/255.f,0}},
            {.position={(rule_x+rule_width)*scale,(l.panel.y+19)*scale},.color={118/255.f,95/255.f,52/255.f,0}},
            {.position={rule_x*scale,(l.panel.y+19)*scale},.color={118/255.f,95/255.f,52/255.f,1}}};
        const int indices[]={0,1,2,0,2,3};
    char stage_title[150];
    if(v->owner==SV_BIRTH_STATS && v->stats_point_buy)
        snprintf(stage_title,sizeof(stage_title),"Distribute your attribute points — use them all %d left",v->stats_remaining);
    else SDL_strlcpy(stage_title,titles[v->owner],sizeof(stage_title));
    if(!SDL_RenderGeometry(r,NULL,rule,4,indices,6) || !b2_birth_summary(r,input,l,scale) ||
       !sv_login_shell_heading(r,stage_title,l.heading.x,l.heading.y,22,scale,b2_normal))return false;
    char intro[140];
    if(waiting)snprintf(intro,sizeof(intro),"Waiting for authoritative play acknowledgement...");
    else if(v->owner==SV_BIRTH_TRAIT)snprintf(intro,sizeof(intro),"%s",sv_birth_trait_intro);
    else if(v->owner==SV_BIRTH_BODY)snprintf(intro,sizeof(intro),"%s",sv_birth_body_intro);
    else if(v->owner==SV_BIRTH_SEX)snprintf(intro,sizeof(intro),"%s",sv_birth_sex_intro);
    else if(v->owner==SV_BIRTH_MODE)snprintf(intro,sizeof(intro),"%s",sv_birth_mode_intro);
    else if(v->owner==SV_BIRTH_STATS && v->stats_point_buy) {
        if(v->stats_confirmation_pending)snprintf(intro,sizeof(intro),"%d unused points. Confirm again to continue.",v->stats_remaining);
        else SDL_strlcpy(intro,"If any values are shown under 'recommended minimum', try to reach those.",sizeof(intro));
    }
    else if(v->owner==SV_BIRTH_STATS)snprintf(intro,sizeof(intro),"Choose stat priority %u of 6. Enter accepts the highlighted attribute.",v->stats_order_count+1);
    else snprintf(intro,sizeof(intro),"Select a choice, then press Enter or Continue. Double-click also accepts.");
    if(v->owner!=SV_BIRTH_CLASS && v->owner!=SV_BIRTH_RACE && !b2_wrap(r,intro,(SvLoginRect){l.heading.x,l.heading.y+33,l.heading.w,0},(v->owner==SV_BIRTH_TRAIT || v->owner==SV_BIRTH_SEX || v->owner==SV_BIRTH_BODY || v->owner==SV_BIRTH_MODE)?15:v->owner==SV_BIRTH_STATS?13.5f:11,scale,v->stats_confirmation_pending?b2_error:b2_muted))return false;
    if(v->owner==SV_BIRTH_CLASS && !b2_class_guidance(r,l,scale))return false;
    if(v->owner==SV_BIRTH_STATS && v->stats_point_buy) {
        SvLoginRect header={l.choices.x,l.choices.y-36,l.choices.w-l.choice_gap,36};
        const char *labels[]={"ATTRIBUTE","BASE",v->setup->classes[v->chosen_class].title,
            v->setup->races[v->chosen_race].title,"","CURRENT","","RECOMMENDED MIN"};
        if(!b2_box(r,(SvLoginRect){header.x,header.y+header.h-1,header.w,1},scale,(SDL_Color){75,71,49,255},false))return false;
        for(unsigned i=0;i<8;++i)if(i!=4 && i!=6) {
            SvLoginRect cell=sv_birth_stat_cell(header,i);cell.y+=8;cell.h=20;
            char title[100];SDL_strlcpy(title,labels[i],sizeof(title));for(char *c=title;*c;++c)if(*c>='a' && *c<='z')*c-=32;
            if(!b2_character_text(r,title,cell,header.w<800?10:12,0,scale,(SDL_Color){143,148,138,255},false,i!=0,false))return false;
        }
    }
    if(!b2_birth_choices_clip(r,l,v->owner,scale))return false;
    bool reference_cards=v->owner==SV_BIRTH_SEX || v->owner==SV_BIRTH_BODY || v->owner==SV_BIRTH_MODE;
    bool ok=true;unsigned count=sv_birth_choice_count(v);int selected=sv_birth_highlight(v);
    const char *detail_title="";
    for(unsigned p=0;p<count;++p)if(sv_birth_choice_id(v,p)==selected)detail_title=sv_birth_choice_title(v,p);
    for(unsigned p=viewport->first;ok && p<count && p<viewport->first+viewport->rows;++p) {
        SvLoginRect row=sv_birth_choice_rect(l,p,viewport->first);
        int id=sv_birth_choice_id(v,p);bool legal=sv_birth_choice_legal(v,p),chosen=id==selected;
        SDL_Color border=chosen?b2_gold:viewport->hover==(int)p && legal?(SDL_Color){115,98,56,255}:(SDL_Color){55,59,51,255};
        if(reference_cards)border=chosen?(SDL_Color){198,164,63,255}:border;
        SDL_Color fill=reference_cards?(chosen?(SDL_Color){74,58,16,87}:viewport->hover==(int)p && legal?(SDL_Color){18,22,14,230}:(SDL_Color){8,12,9,194}):chosen?(SDL_Color){44,37,13,235}:(SDL_Color){8,12,9,235};
        bool stat_row=v->owner==SV_BIRTH_STATS && v->stats_point_buy;
        if(stat_row) {
            ok=b2_birth_stat_selection(r,row,scale,chosen,viewport);
        } else ok=((reference_cards || v->owner==SV_BIRTH_CLASS || v->owner==SV_BIRTH_RACE || v->owner==SV_BIRTH_TRAIT)?b2_round(r,row,4,scale,fill,false) && b2_round(r,row,4,scale,border,true):b2_box(r,row,scale,chosen?(SDL_Color){44,37,13,235}:(SDL_Color){8,12,9,235},false) && b2_box(r,row,scale,border,true));

        if(ok && (v->owner==SV_BIRTH_SEX || v->owner==SV_BIRTH_MODE) && chosen)
            ok=b2_box(r,(SvLoginRect){row.x+1,row.y+3,3,row.h-6},scale,(SDL_Color){216,180,67,255},false);
        char caption[160];
        if(v->owner==SV_BIRTH_STATS && v->stats_point_buy) {
            int race=v->chosen_race,cls=v->chosen_class;
            int rb=race>=0 && race<v->setup->race_count?v->setup->races[race].adjustment[id]:0;
            int cb=cls>=0 && cls<v->setup->class_count?v->setup->classes[cls].adjustment[id]:0;
            snprintf(caption,sizeof(caption),"%s  %d  (%+d / %+d)",sv_birth_choice_title(v,p),v->stats[id],cb,rb);
        } else {
            char hotkey=b2_birth_hotkey(v,p);
            if(hotkey)snprintf(caption,sizeof(caption),"%c) %s",hotkey,sv_birth_choice_title(v,p));
            else snprintf(caption,sizeof(caption),"%s",sv_birth_choice_title(v,p));
        }
        unsigned card_lines=reference_cards?sv_birth_card_lines(b2_birth_card_copy(v,id),(unsigned)((row.w-24)/(13.5f*.61f))):0;
        float card_top=reference_cards?(row.h-16.8f-4-card_lines*18.225f)/2:8;
        SvLoginRect text={row.x+12,row.y+card_top,row.w-(v->owner==SV_BIRTH_STATS && v->stats_point_buy?85:24),row.h-10};
        if(ok && v->owner==SV_BIRTH_STATS && v->stats_point_buy)
            ok=b2_birth_stat_row(r,v,(unsigned)id,row,scale) && b2_birth_choices_clip(r,l,v->owner,scale);
        else if(ok)ok=b2_clip(r,text,scale) && b2_text(r,caption,text.x,text.y,v->owner==SV_BIRTH_STATS?13:reference_cards?14:16.5f,scale,!legal?(SDL_Color){85,91,84,255}:reference_cards?(chosen?(SDL_Color){240,210,104,255}:(SDL_Color){208,206,194,255}):chosen?b2_gold:b2_normal) && b2_birth_choices_clip(r,l,v->owner,scale);
        if(ok && (v->owner==SV_BIRTH_SEX || v->owner==SV_BIRTH_BODY || v->owner==SV_BIRTH_MODE)) {
            SvLoginRect copy={row.x+12,row.y+(reference_cards?card_top+20.8f:33),row.w-24,row.h-(reference_cards?card_top+20.8f:36)};
            ok=b2_clip(r,copy,scale) && b2_wrap_leading(r,b2_birth_card_copy(v,id),copy,reference_cards?13.5f:12,scale,reference_cards && chosen?(SDL_Color){170,167,137,255}:b2_muted,reference_cards?1.35f:1.5f) && b2_birth_choices_clip(r,l,v->owner,scale);
        }
    }
    if(!SDL_SetRenderClipRect(r,NULL) || !ok)return false;
    if(count>viewport->rows) {
        char scroll[64];snprintf(scroll,sizeof(scroll),"%u-%u of %u  (wheel / PgUp / PgDn)",viewport->first+1,viewport->first+viewport->rows<count?viewport->first+viewport->rows:count,count);
        if(!b2_text(r,scroll,l.choices.x,l.choices.y+l.choices.h-11,9,scale,b2_muted))return false;
    }
    if(l.detail.h>0) {
        const char *copy=v->owner==SV_BIRTH_BODY?b2_birth_card_copy(v,selected):b2_birth_description(v->owner,detail_title);
        bool modifiers=v->setup && selected>=0 &&
            ((v->owner==SV_BIRTH_CLASS && selected<v->setup->class_count) ||
             (v->owner==SV_BIRTH_RACE && selected<v->setup->race_count));
        float title_width=0;
        if(!sv_login_shell_measure(r,detail_title,22,scale,true,&title_width))return false;
        float title_height=b2_wrap_count(detail_title,(unsigned)((l.detail.w-36)/(22*.61f)))*33;
        bool inline_mods=modifiers && title_width+18+368<=l.detail.w-28;
        float header_height=inline_mods?26:title_height+(modifiers?30:0);
        bool reference_copy=modifiers || v->owner==SV_BIRTH_TRAIT || v->owner==SV_BIRTH_STATS;
        float body_size=reference_copy?16.5f:15;
        float content=header_height+40+b2_wrap_count(copy,(unsigned)((l.detail.w-36)/(body_size*.61f)))*body_size*1.5f;
        viewport->detail_max=content>l.detail.h?(unsigned)(content-l.detail.h):0;
        if(viewport->detail_scroll>viewport->detail_max)viewport->detail_scroll=viewport->detail_max;
        if(!b2_box(r,(SvLoginRect){l.detail.x,l.detail.y,l.detail.w,1},scale,(SDL_Color){128,109,56,184},false) || !b2_clip(r,l.detail,scale))return false;
        float y=l.detail.y+18-viewport->detail_scroll;
        ok=b2_birth_detail_title(r,detail_title,(SvLoginRect){l.detail.x+18,y,l.detail.w-36,title_height},scale);
        if(ok && modifiers) {
            const int16_t *mods=v->owner==SV_BIRTH_CLASS?v->setup->classes[selected].adjustment:v->setup->races[selected].adjustment;
            static const char *labels[]={"Str","Int","Wis","Dex","Con","Cha"};
            float x=l.detail.x+18+(inline_mods?title_width+18:0),my=y+(inline_mods?1:title_height+4);
            for(unsigned i=0;ok && i<6;++i) {
                char number[16];SDL_snprintf(number,sizeof(number),"%+d",mods[i]);
                SvLoginRect badge={x+i*62,my,58,24};
                ok=b2_box(r,badge,scale,(SDL_Color){77,73,49,122},true) &&
                    b2_text(r,labels[i],badge.x+6,badge.y+5,12,scale,(SDL_Color){135,140,130,255}) &&
                    b2_character_text(r,number,(SvLoginRect){badge.x+28,badge.y+5,25,18},12,0,scale,
                        mods[i]<0?(SDL_Color){217,121,100,255}:(SDL_Color){223,196,93,255},false,false,true);
            }
        }
        if(ok)ok=b2_wrap(r,copy,(SvLoginRect){l.detail.x+18,y+header_height+12,l.detail.w-36,0},body_size,scale,
            reference_copy?(SDL_Color){199,167,120,255}:b2_normal);
        if(!SDL_SetRenderClipRect(r,NULL) || !ok)return false;
    }
    char keys[300];SDL_strlcpy(keys,waiting?"Waiting for server acknowledgement · Esc quit":
        v->owner==SV_BIRTH_STATS && v->stats_point_buy?"Up/Down select · +/- adjust · Enter/Esc confirm · Backspace back":
        (v->owner==SV_BIRTH_SEX || v->owner==SV_BIRTH_BODY || v->owner==SV_BIRTH_CLASS || v->owner==SV_BIRTH_RACE || v->owner==SV_BIRTH_TRAIT)?
        "Letter selects+advances · Arrows highlight · Enter · Backspace back":
        "Arrows select · Enter confirm · Backspace back",sizeof(keys));
    if(!waiting) {
        if(sv_birth_action_available(v,SV_BIRTH_RANDOM))SDL_strlcat(keys," · * random",sizeof(keys));
        if(sv_birth_action_available(v,SV_BIRTH_DNA))SDL_strlcat(keys," · # DNA",sizeof(keys));
        if(sv_birth_action_available(v,SV_BIRTH_AUTOMATIC_DNA))SDL_strlcat(keys," · % Auto",sizeof(keys));
        if(v->owner>=SV_BIRTH_CLASS && !(v->owner==SV_BIRTH_STATS && !v->stats_point_buy))SDL_strlcat(keys," · ? guide",sizeof(keys));
        SDL_strlcat(keys," · Ctrl+Q quit",sizeof(keys));
    }
    SvLoginRect key_box=footer.h>46?(SvLoginRect){footer.x+20,footer.y+8,footer.w-40,18}:
        (SvLoginRect){l.back.x+l.back.w+16,l.back.y+9,l.confirm.x-l.back.x-l.back.w-26,18};
    if(!b2_character_text(r,keys,key_box,12,0,scale,(SDL_Color){114,120,111,255},false,false,true) ||
       !b2_character_action(r,l.back,"BACK",scale,viewport->focus==1,sv_birth_action_available(v,SV_BIRTH_BACK),false))return false;
    if(waiting) {
        SvLoginRect card={l.heading.x,l.choices.y,l.choices.w,l.choices.h};
        return b2_state_frame(r,card,card,"Please wait",scale,false) &&
            b2_status(r,input->contact_status,(SvLoginRect){card.x+22,card.y+74,card.w-44,card.h-90},scale,false) &&
            b2_character_action(r,l.confirm,"WAIT...",scale,false,false,true);
    }
    ok=b2_character_action(r,l.confirm,v->stats_confirmation_pending?"CONFIRM":v->owner==SV_BIRTH_MODE?"CREATE & ENTER":"CONTINUE",scale,viewport->focus==7,true,true);
    if(!ok || !v->stats_confirmation_pending)return ok;
    return b2_state_frame(r,l.panel,l.warning,"Unused attribute points",scale,false) &&
        b2_wrap(r,intro,(SvLoginRect){l.warning.x+22,l.warning.y+67,l.warning.w-44,0},13,scale,b2_normal) &&
        b2_button(r,l.warning_cancel,"Keep editing",scale,viewport->focus==1,true) &&
        b2_primary_button(r,l.warning_confirm,"Continue",scale,viewport->focus!=1);
}

bool sv_endpoint_render(SDL_Renderer *renderer, SvFont *font,
                        const SvEndpoint *endpoint, const SvEndpointInput *input)
{
    /* C is whole-window and has priority over all shell D adapters. */
    if(!input->temporary_surface_text && !input->contact_failed && input->login_view && input->login_view->motd) {
        SvMotd motd=sv_motd_prepare(input->login_view->motd,input->login_view->motd_size);
        return sv_login_shell_motd(renderer,&motd);
    }
    if(input->temporary_surface_text && input->birth_view && input->birth_viewport) {
        if(!b2_birth(renderer,endpoint,input))return false;
        int w,h;if(!SDL_GetCurrentRenderOutputSize(renderer,&w,&h))return false;
        float scale=SDL_GetWindowDisplayScale(SDL_GetRenderWindow(renderer));if(scale<=0)scale=1;
        SvBirthLayout l=sv_birth_layout(w/scale,h/scale,input->birth_view->owner);
        return b2_state_frame(renderer,l.panel,l.warning,"Guide",scale,false) &&
            b2_wrap(renderer,input->temporary_surface_text,(SvLoginRect){l.warning.x+22,l.warning.y+67,l.warning.w-44,0},13,scale,b2_normal) &&
            b2_button(renderer,l.warning_confirm,"Return",scale,true,true);
    }
    if(!input->temporary_surface_text && !input->contact_failed && input->birth_view && input->birth_viewport)
        return b2_birth(renderer,endpoint,input);
    if(!input->temporary_surface_text && !input->contact_failed && input->login_view && (input->login_view->overview || input->login_view->ordinary_name_entry))
        return b2_characters(renderer,font,endpoint,input);
    if(!input->temporary_surface_text && input->login_view && input->login_view->live_handoff) {
        if(!sv_login_shell_render(renderer,endpoint,input))return false;
        int w,h;if(!SDL_GetCurrentRenderOutputSize(renderer,&w,&h))return false;
        float scale=SDL_GetWindowDisplayScale(SDL_GetRenderWindow(renderer));if(scale<=0)scale=1;
        SvLoginLayout l=sv_login_layout(w/scale,h/scale,0,0);
        SvLoginRect card={l.panel.x+24,l.panel.y+24,l.panel.w-48,l.panel.h-48};
        return b2_state_frame(renderer,l.panel,card,"Startup screens complete",scale,false) &&
            b2_wrap(renderer,"Waiting for live-session handoff (Stage C).",(SvLoginRect){card.x+22,card.y+74,card.w-44,0},15,scale,b2_normal);
    }
    if (endpoint->phase == SV_ENDPOINT_LIST && !input->temporary_surface_text)
        return sv_login_shell_render(renderer, endpoint,input);
    if (!input->temporary_surface_text && (endpoint->phase == SV_ENDPOINT_MANUAL ||
        (endpoint->phase == SV_ENDPOINT_SELECTED && (input->credentials || input->contact_failed ||
         !input->login_view || (!input->login_view->overview && !input->login_view->birth && !input->login_view->motd)))))
        return b2_forms(renderer,font,endpoint,input);
    if(!sv_login_shell_render(renderer,endpoint,input))return false;
    int width,height;
    if(!SDL_GetCurrentRenderOutputSize(renderer,&width,&height))return false;
    float scale=SDL_GetWindowDisplayScale(SDL_GetRenderWindow(renderer));if(scale<=0)scale=1;
    SvLoginLayout l=sv_login_layout(width/scale,height/scale,0,0);
    SvLoginRect card={l.panel.x+24,l.panel.y+24,l.panel.w-48,l.panel.h-48};
    return b2_state_frame(renderer,l.panel,card,input->temporary_surface_text?"Guide":"Please wait",scale,false) &&
        b2_status(renderer,input->temporary_surface_text?input->temporary_surface_text:input->contact_status,
            (SvLoginRect){card.x+22,card.y+74,card.w-44,card.h-96},scale,false);
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
