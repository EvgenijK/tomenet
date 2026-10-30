#ifndef SV_UI_LOGIN_LAYOUT_H
#define SV_UI_LOGIN_LAYOUT_H

/* B2 rectangles are recomputed from current logical dimensions on each use.
 * Render output = logical * display scale; window events additionally include
 * pixel density. No stored rectangles survive geometry or model changes. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "input/endpoint.h"
#include "input/birth-interaction.h"
typedef struct { float x, y, w, h; } SvLoginRect;
typedef struct {
    SvLoginRect panel, chronicle, links, logo, list, manual;
    float row_height;
    size_t first, rows;
    uint32_t group_headers;
    int compact, character_name_entry;
} SvLoginLayout;
static inline SvLoginLayout sv_login_layout(float width, float height,
                                             size_t count, size_t selected)
{
    SvLoginLayout l = {0};
    l.compact = width <= 1240;
    float panel_width = width - 414;
    if (panel_width < 400) panel_width = 400;
    if (panel_width > 860) panel_width = 860;
    float hero = l.compact ? width * .1125f + 21 : 250;
    float top = height <= 800 ? 8 : 12;
    float bottom = height <= 800 ? 8 : 14;
    float gap = height <= 800 ? 8 : 10;
    if (l.compact && hero > 160) hero = 160;
    if (!l.compact && height <= 800) hero = 190;
    l.panel = (SvLoginRect){(width-panel_width)/2, top+hero+gap,
                           panel_width, height-top-hero-gap-bottom-(l.compact?40+gap:0)};
    l.logo = (SvLoginRect){(width-(width<1184?width-54:1160))/2,
                           top+(hero-(width<1184?width-54:1160)*144/1280)/2,
                           width<1184?width-54:1160,
                           (width<1184?width-54:1160)*144/1280};
    float side = (width-panel_width)/2-27;
    if (side>340) side=340;
    l.chronicle = (SvLoginRect){l.panel.x-12-side,l.panel.y,side,l.panel.h};
    l.links = (SvLoginRect){l.compact?(width-290)/2:l.panel.x+panel_width+46,
                           l.compact?height-bottom-40:l.panel.y+28,
                           l.compact?290:side-34,l.compact?40:360};
    l.manual = (SvLoginRect){l.panel.x+24,l.panel.y+l.panel.h-52,panel_width-48,32};
    l.list = (SvLoginRect){l.panel.x+21,l.panel.y+(height<=800?65:91),panel_width-42,l.panel.h-(height<=800?129:155)};
    l.row_height = height<=800?72:88;
    l.rows = l.list.h>0?(size_t)(l.list.h/(l.row_height+10)):0;
    if (!l.rows) l.rows=1;
    l.first = selected<count && selected>=l.rows?selected-l.rows+1:0;
    return l;
}
static inline SvLoginRect sv_login_row(SvLoginLayout l, size_t row)
{
    float y=l.list.y;
    for(size_t i=l.first;i<=row;++i) {
        if(l.group_headers & (UINT32_C(1)<<i))y+=30;
        if(i<row)y+=l.row_height+9;
    }
    return (SvLoginRect){l.list.x,y,l.list.w,l.row_height};
}
/* Presentation classification uses the exact published official hosts; an
 * arbitrary name ending in tomenet.eu is not an official server identity. */
static inline int sv_login_server_official(const SvServer *server)
{
    return !strcmp(server->host,"europe.tomenet.eu") ||
           !strcmp(server->host,"na.tomenet.eu") ||
           !strcmp(server->host,"apac.tomenet.eu");
}
/* The hint paragraph uses fixed logical monospace advances. Raster font
 * hinting at fractional DPI must not change wrapping or input rectangles. */
static inline float sv_login_hint_piece_width(unsigned piece)
{
    static const unsigned chars[]={1,1,1,10,1,1,1,19,5,17,1,22};
    static const unsigned keys[]={1,0,1,0,1,0,1,0,1,0,1,0};
    return chars[piece]*6.f+keys[piece]*12.f;
}
static inline float sv_login_hint_width(unsigned first,unsigned end)
{
    float total=0;for(unsigned i=first;i<end;++i)total+=sv_login_hint_piece_width(i);
    return total;
}
static inline SvLoginLayout sv_login_server_layout(float width,float height,const SvEndpoint *endpoint)
{
    SvLoginLayout l=sv_login_layout(width,height,endpoint->server_count,endpoint->selected);
    l.list.y=l.panel.y+(height<=800?51:58);
    /* Direct is the inline Q hint in the final D shortcut paragraph. The
     * fixed 10-unit monospace role has a six-unit advance. */
    float hint_width=sv_login_hint_width(0,12);
    bool wrap_hints=l.panel.w-42<hint_width;
    float hint_y=l.panel.y+(l.row_height<=72?14:26);
    l.manual=(SvLoginRect){l.panel.x+(l.panel.w-(wrap_hints?sv_login_hint_width(8,12):hint_width))/2+
                           sv_login_hint_width(wrap_hints?8:0,10),hint_y+(wrap_hints?20:0)-2,150,15};
    l.list.h=l.panel.y+l.panel.h-l.list.y-18;
    for(size_t i=0;i<endpoint->server_count;++i)
        if(!i || sv_login_server_official(&endpoint->servers[i])!=sv_login_server_official(&endpoint->servers[i-1]))
            l.group_headers|=UINT32_C(1)<<i;
    /* Keep slot order and hotkeys intact, including interleaved server kinds.
     * Every visible run receives its own section caption. */
    l.first=0;
    for(;;) {
        l.group_headers|=UINT32_C(1)<<l.first;
        l.rows=0;
        for(size_t i=l.first;i<endpoint->server_count;++i) {
            SvLoginRect row=sv_login_row(l,i);
            if(row.y+row.h>l.list.y+l.list.h && l.rows)break;
            ++l.rows;
        }
        if(!l.rows)l.rows=1;
        if(endpoint->selected>=endpoint->server_count || endpoint->selected<l.first+l.rows || l.first+1>=endpoint->server_count)break;
        ++l.first;
    }
    return l;
}
static inline SvLoginRect sv_login_server_door(SvLoginRect row)
{ return (SvLoginRect){row.x+row.w-47,row.y+(row.h-34)/2,34,34}; }
/* Account and modal controls use the same rectangles for paint and input. */
typedef struct {
    SvLoginRect account, password, reveal, submit, back, status, guide, account_back;
    SvLoginRect modal, address, connect, cancel;
} SvLoginFormLayout;
static inline SvLoginFormLayout sv_login_form_layout(SvLoginLayout l, float height)
{
    SvLoginFormLayout f = {0};
    float width = l.panel.w-80;
    if (width>520) width=520;
    float x=l.panel.x+(l.panel.w-width)/2, y=l.panel.y+(height<=800?156:168);
    f.account=(SvLoginRect){x,y,width,40};
    f.password=(SvLoginRect){x,y+94,width,40};
    f.reveal=(SvLoginRect){x+width-58,y+94,48,40};
    f.submit=(SvLoginRect){x,y+170,width,32};
    f.back=(SvLoginRect){l.panel.x+20,l.panel.y+l.panel.h-42,100,30};
    f.account_back=(SvLoginRect){l.panel.x+20,l.panel.y+l.panel.h-39.5f,58,32};
    f.status=(SvLoginRect){x,y+310,width,42};
    f.guide=(SvLoginRect){x,f.submit.y+108.5f,192,15.5f};
    float mw=390;
    f.modal=(SvLoginRect){l.panel.x+(l.panel.w-mw)/2,l.panel.y+l.panel.h/2-207,mw,270};
    f.address=(SvLoginRect){f.modal.x+19,f.modal.y+113,mw-38,40};
    f.cancel=(SvLoginRect){f.modal.x+19,f.modal.y+215,88,36};
    f.connect=(SvLoginRect){f.modal.x+mw-117,f.modal.y+215,98,36};
    return f;
}
static inline SvLoginLayout sv_character_shell_layout(float width,float height,int name_entry)
{
    SvLoginLayout l=sv_login_layout(width,height,0,0);
    l.character_name_entry=name_entry;
    if(name_entry) {
        l.panel.w=width-30;if(l.panel.w>1480)l.panel.w=1480;
        l.panel.x=(width-l.panel.w)/2;
        /* Creation has no compact realm-link footer. */
        if(l.compact)l.panel.h+=40+(height<=800?8:10);
    }
    return l;
}
/* Character rows retain server order. first is a presentation scroll offset. */
static const char sv_name_intro_text[]="Name your new adventurer. Leave the field blank and choose Random for inspiration.";
static inline unsigned sv_name_intro_lines(float width)
{
    unsigned columns=(unsigned)(width/(15*.61f));if(!columns)columns=1;
    size_t at=0,length=sizeof(sv_name_intro_text)-1;unsigned lines=0;
    while(at<length) {
        size_t count=length-at;if(count>columns)count=columns;
        if(at+count<length) {size_t split=count;while(split && sv_name_intro_text[at+split]!=' ')--split;if(split)count=split;}
        at+=count;while(sv_name_intro_text[at]==' ')++at;++lines;
    }
    return lines;
}
typedef struct {
    SvLoginRect list, ordinary, exclusive, enter, quit, summary, name, random, back;
    SvLoginRect footer, status, hint;
    size_t first, rows, characters;
} SvCharacterLayout;
static inline SvCharacterLayout sv_character_layout(SvLoginLayout shell, size_t first)
{
    SvCharacterLayout c={0};
    SvLoginRect p=shell.panel;
    c.list=(SvLoginRect){p.x+22,p.y+130,p.w-44,p.h-215};
    c.rows=c.list.h>0?(size_t)(c.list.h/35):1;
    if(!c.rows)c.rows=1;
    c.first=first;
    float y=p.y+p.h-39;
    c.quit=(SvLoginRect){p.x+20,y,70,32};
    c.ordinary=(SvLoginRect){p.x+102,y,172,32};
    c.exclusive=(SvLoginRect){p.x+282,y,196,32};
    c.enter=(SvLoginRect){p.x+p.w-118,y,98,32};
    c.summary=(SvLoginRect){p.x+22,p.y+70,p.w<1100?288:340,p.h-143};
    c.name=(SvLoginRect){p.x+(p.w<1100?327:379),p.y+165,p.w-(p.w<1100?371:423),40};
    if(c.name.w>466)c.name.w=466;
    c.random=(SvLoginRect){c.name.x+c.name.w+8,c.name.y,86,40};
    c.back=c.quit;
    if(shell.character_name_entry) {
        bool short_height=shell.row_height<=72;
        float side=shell.compact?290:340,stage=p.x+side+(shell.compact?37:41);
        float intro_lines=sv_name_intro_lines(p.w-side-(shell.compact?60:64));
        c.summary=(SvLoginRect){p.x+21,p.y+70,side,p.h-135};
        c.name=(SvLoginRect){stage,p.y+(short_height?145.25f:164.1f)+(intro_lines-1)*(short_height?20.25f:22.5f),466,40};
        c.random=(SvLoginRect){c.name.x+c.name.w+8,c.name.y,86,40};
        c.footer=(SvLoginRect){p.x,p.y+p.h-46,p.w,46};
        c.back=(SvLoginRect){p.x+21,c.footer.y+7,58,32};
        c.enter=(SvLoginRect){p.x+p.w-107,c.footer.y+7,86,32};
        c.hint=(SvLoginRect){c.enter.x-320,c.enter.y+9,310,16};
    } else {
        float footer_height=p.w<800?70:46;
        c.footer=(SvLoginRect){p.x,p.y+p.h-footer_height,p.w,footer_height};
        float button_y=c.footer.y+7;
        c.quit=(SvLoginRect){p.x+20,button_y,58,32};
        c.ordinary=(SvLoginRect){p.x+88,button_y,157,32};
        c.exclusive=(SvLoginRect){p.x+255,button_y,177,32};
        c.enter=(SvLoginRect){p.x+p.w-78,button_y,58,32};
        c.status=p.w<800?(SvLoginRect){p.x+20,button_y+39,p.w-40,16}:
            (SvLoginRect){p.x+442,button_y+9,p.w-644,16};
        c.hint=p.w<800?(SvLoginRect){0,0,0,0}:
            (SvLoginRect){p.x+p.w-190,button_y+10,102,14};
        c.list=(SvLoginRect){p.x+20,p.y+128,p.w-40,c.footer.y-p.y-138};
    }
    return c;
}
static inline SvCharacterLayout sv_character_content_layout(SvCharacterLayout c,size_t characters)
{
    c.characters=characters;
    c.rows=0;
    float used=0;
    while(used+(c.first+c.rows<characters?28:40)<=c.list.h) {
        used+=c.first+c.rows<characters?35:47;
        ++c.rows;
    }
    if(!c.rows)c.rows=1;
    return c;
}
static inline SvLoginRect sv_character_row(SvCharacterLayout c,size_t slot)
{
    size_t first_placeholder=c.first>c.characters?c.first:c.characters;
    size_t placeholders=slot>first_placeholder?slot-first_placeholder:0;
    return (SvLoginRect){c.list.x,c.list.y+(slot-c.first)*35+placeholders*12,c.list.w-5,slot<c.characters?28:40};
}
/* Birth presentation: summary left, independent choices/detail viewports.
 * The interaction owns IDs and ordering; these rectangles own only geometry. */
typedef struct {
    SvLoginRect panel, summary, heading, choices, detail, footer;
    SvLoginRect back, quit, random, dna, automatic, guide, confirm;
    SvLoginRect warning, warning_cancel, warning_confirm;
    unsigned columns, rows;
    float row_height, choice_gap;
} SvBirthLayout;
static const char *const sv_class_guidance_prefix[]={
    "Close-combat classes (especially warrior) are recommended for beginners.",
    "Ranged-combat classes are harder for beginners:","Not recommended for beginners:"};
static const char *const sv_class_guidance_suffix[]={""," Archer and Ranger."," Istar, Priest, Adventurer, Shaman, Runemaster."};
static inline float sv_class_guidance_height(float width)
{
    unsigned columns=(unsigned)((width-26)/(15*.61f));if(!columns)columns=1;
    unsigned lines=0;
    for(unsigned i=0;i<3;++i) {
        char text[160];size_t used=0;
        const char *parts[]={sv_class_guidance_prefix[i],sv_class_guidance_suffix[i]};
        for(unsigned part=0;part<2;++part)for(size_t n=0;parts[part][n] && used+1<sizeof(text);++n)text[used++]=parts[part][n];
        text[used]=0;size_t at=0;
        while(at<used) {
            size_t count=used-at;if(count>columns)count=columns;
            if(at+count<used){size_t split=count;while(split && text[at+split]!=' ')--split;if(split)count=split;}
            at+=count;while(text[at]==' ')++at;++lines;
        }
    }
    return 22+lines*22.5f+10;
}
static const char sv_birth_trait_intro[]="Some races have a lineage or spiritual path; others continue without one.";
static const char sv_birth_sex_intro[]="This choice identifies the character and does not restrict classes or races.";
static const char sv_birth_body_intro[]="A body modification changes important equipment and movement rules.";
static const char sv_birth_mode_intro[]="Game mode determines lives, social restrictions and victory rules.";
static const char sv_birth_normal_copy[]="Use the normal body and equipment slots of the selected race.";
static const char sv_birth_bat_copy[]="Faster and vampiric, but unable to wear some equipment. Not recommended for shapeshifters or Archers.";
static inline unsigned sv_birth_card_lines(const char *text,unsigned columns)
{
    unsigned lines=0;size_t at=0;if(!columns)columns=1;
    while(text[at]) {
        size_t count=strlen(text+at);if(count>columns)count=columns;
        if(text[at+count]) {size_t split=count;while(split && text[at+split]!=' ')--split;if(split)count=split;}
        at+=count;while(text[at]==' ')++at;++lines;
    }
    return lines;
}
static inline SvBirthLayout sv_birth_layout(float width,float height,SvBirthOwner owner)
{
    SvBirthLayout b={.choice_gap=8};
    b.panel=sv_character_shell_layout(width,height,1).panel;
    SvLoginRect p=b.panel;
    float side=width<=1240?290:340,footer_height=width<=1240?70:46;
    b.footer=(SvLoginRect){p.x,p.y+p.h-footer_height,p.w,footer_height};
    b.summary=(SvLoginRect){p.x+21,p.y+70,side,p.h-135-(footer_height-46)};
    float x=b.summary.x+side+17,w=p.w-side-61;
    b.heading=(SvLoginRect){x,p.y+77,w,57};
    float available=p.h-220;
    /* Class/race/trait/stats carry native detail; sex/body/mode use cards. */
    int detail=owner>=SV_BIRTH_CLASS && owner<=SV_BIRTH_STATS && owner!=SV_BIRTH_BODY;
    float list_height=detail?(available-14)*.6f:available;
    b.choices=(SvLoginRect){x,p.y+134,w,list_height};
    b.detail=(SvLoginRect){x,p.y+134+list_height+14,w,detail?(available-14)*.4f:0};
    b.columns=(owner==SV_BIRTH_STATS || (width<=1240 && (owner==SV_BIRTH_RACE || owner==SV_BIRTH_TRAIT)))?1:owner==SV_BIRTH_SEX || owner==SV_BIRTH_BODY || owner==SV_BIRTH_MODE?2:owner==SV_BIRTH_CLASS?(width>1600?4:2):(w>800?3:2);
    b.row_height=owner==SV_BIRTH_STATS?36:(owner==SV_BIRTH_SEX || owner==SV_BIRTH_BODY || owner==SV_BIRTH_MODE)?94:44;
    if(owner==SV_BIRTH_CLASS) {
        b.heading.y=p.y+74;b.choices.y=p.y+112+sv_class_guidance_height(w)+14;
        b.choices.h=b.detail.y-b.choices.y-14;list_height=b.choices.h;b.row_height=50;
        b.detail.h=b.footer.y-20-b.detail.y;
    }
    if(owner==SV_BIRTH_RACE || owner==SV_BIRTH_TRAIT) {
        b.heading.y=p.y+74;
        unsigned trait_lines=(unsigned)(w/(15*.61f))<sizeof(sv_birth_trait_intro)-1?2:1;
        b.choices.y=p.y+(owner==SV_BIRTH_RACE?112:142+(trait_lines-1)*22.5f);
        b.choices.h=b.detail.y-b.choices.y-14;list_height=b.choices.h;b.row_height=50;
        b.detail.h=b.footer.y-20-b.detail.y;
    }
    if(owner==SV_BIRTH_SEX || owner==SV_BIRTH_BODY || owner==SV_BIRTH_MODE) {
        b.heading.y=p.y+74;b.heading.x+=3;b.heading.w-=3;
        b.choices.x+=3;b.choices.w+=5;w-=3;
        unsigned columns=(unsigned)(w/(15*.61f));if(!columns)columns=1;
        unsigned lines=sv_birth_card_lines(owner==SV_BIRTH_SEX?sv_birth_sex_intro:owner==SV_BIRTH_BODY?sv_birth_body_intro:sv_birth_mode_intro,columns);
        b.choices.y=b.heading.y+26+7+lines*22.5f+18;
        b.choices.h=b.footer.y-20-b.choices.y;list_height=b.choices.h;
        /* Shared grid row grows to fit the longer description, as in CSS. */
        unsigned copy_columns=(unsigned)((w/2-32)/(13.5f*.61f));if(!copy_columns)copy_columns=1;
        unsigned copy_lines=owner==SV_BIRTH_SEX?1:sv_birth_card_lines(owner==SV_BIRTH_BODY?sv_birth_bat_copy:"Special player-versus-player rules replace normal victory.",copy_columns);
        float card_height=20+16.8f+4+copy_lines*18.225f;
        b.row_height=(card_height>58?card_height:58)+8;
    }
    if(owner==SV_BIRTH_STATS) {
        b.heading.y=p.y+74;
        b.choices.y=p.y+194;b.choices.h=b.detail.y-b.choices.y-14;
        list_height=b.choices.h;b.row_height=58;b.choice_gap=0;
        b.detail.h=b.footer.y-20-b.detail.y;
    }
    b.rows=list_height>0?(unsigned)(list_height/b.row_height):1;
    if(!b.rows)b.rows=1;
    float y=p.y+p.h-39;
    b.back=(SvLoginRect){p.x+20,y,58,32};
    b.confirm=(SvLoginRect){p.x+p.w-107,y,86,32};
    if(owner==SV_BIRTH_MODE)b.confirm=(SvLoginRect){p.x+p.w-148,y,127,32};
    b.warning=(SvLoginRect){p.x+(p.w-480)/2,p.y+(p.h-190)/2,480,190};
    b.warning_cancel=(SvLoginRect){b.warning.x+22,b.warning.y+136,150,32};
    b.warning_confirm=(SvLoginRect){b.warning.x+306,b.warning.y+136,150,32};
    return b;
}
static inline SvLoginRect sv_birth_choice_rect(SvBirthLayout b,unsigned position,unsigned first)
{
    unsigned offset=position-first;
    float cell=b.choices.w/b.columns;
    return (SvLoginRect){b.choices.x+(offset%b.columns)*cell,
        b.choices.y+(offset/b.columns)*b.row_height,cell-b.choice_gap,b.row_height-b.choice_gap};
}
/* Columns and +/- hit boxes share the reference table geometry. */
static inline SvLoginRect sv_birth_stat_cell(SvLoginRect row,unsigned column)
{
    int compact=row.w<800;float gap=compact?2:4;
    float widths[8]={0,compact?44:58,compact?58:72,compact?58:72,
                      compact?14:18,compact?78:90,compact?54:60,0};
    float flex=row.w-20-gap*7;
    for(unsigned i=1;i<7;++i)flex-=widths[i];
    widths[0]=flex/1.9f;widths[7]=flex-widths[0];
    float x=row.x+10;for(unsigned i=0;i<column;++i)x+=widths[i]+gap;
    return (SvLoginRect){x,row.y,widths[column],row.h};
}
static inline SvLoginRect sv_birth_stat_minus(SvLoginRect row)
{ SvLoginRect cell=sv_birth_stat_cell(row,6);return (SvLoginRect){cell.x+(cell.w-54)/2,row.y+(row.h-25)/2,25,25}; }
static inline SvLoginRect sv_birth_stat_plus(SvLoginRect row)
{ SvLoginRect cell=sv_birth_stat_minus(row);cell.x+=29;return cell; }
/* Realm circles and labels share one pointer target. */
static inline SvLoginRect sv_login_link_rect(SvLoginLayout l,size_t index)
{
    float size=l.compact?40:48;
    return (SvLoginRect){l.links.x+(l.compact?index*50:0),
        l.links.y+(l.compact?0:index*61),size,size};
}
static inline SvLoginRect sv_login_link_hit(SvLoginLayout l,size_t index)
{
    static const size_t label_lengths[]={15,12,11,14,17,6};
    SvLoginRect r=sv_login_link_rect(l,index);
    if(!l.compact)r.w=59+label_lengths[index]*6.2f;
    return r;
}
static inline int sv_login_contains(SvLoginRect r,float x,float y)
{ return x>=r.x && y>=r.y && x<r.x+r.w && y<r.y+r.h; }

#endif
