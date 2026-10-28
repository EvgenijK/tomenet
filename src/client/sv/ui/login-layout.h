#ifndef SV_UI_LOGIN_LAYOUT_H
#define SV_UI_LOGIN_LAYOUT_H

/* Logical coordinates shared by the native startup scene and hit testing. */
enum {
    SV_LOGIN_ROW_TOP = 275,
    SV_LOGIN_ROW_HEIGHT = 25,
    SV_LOGIN_NAME_TOP = 215,
    SV_LOGIN_NAME_HINT_TOP = 245,
    SV_LOGIN_NAME_BOTTOM = 240
};


/* B2 rectangles are recomputed from current logical dimensions on each use.
 * Render output = logical * display scale; window events additionally include
 * pixel density. No stored rectangles survive geometry or model changes. */
#include <stddef.h>
typedef struct { float x, y, w, h; } SvLoginRect;
typedef struct {
    SvLoginRect panel, chronicle, links, logo, list, manual;
    float row_height;
    size_t first, rows;
    int compact;
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
    return (SvLoginRect){l.list.x,l.list.y+(row-l.first)*(l.row_height+10),l.list.w,l.row_height};
}
/* Account and modal controls use the same rectangles for paint and input. */
typedef struct {
    SvLoginRect account, password, reveal, submit, back, status;
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
    f.reveal=(SvLoginRect){x+width-66,y+94,66,40};
    f.submit=(SvLoginRect){x,y+170,width,32};
    f.back=(SvLoginRect){l.panel.x+20,l.panel.y+l.panel.h-42,100,30};
    f.status=(SvLoginRect){x,y+310,width,42};
    float mw=390;
    f.modal=(SvLoginRect){l.panel.x+(l.panel.w-mw)/2,l.panel.y+l.panel.h/2-207,mw,270};
    f.address=(SvLoginRect){f.modal.x+19,f.modal.y+113,mw-38,40};
    f.cancel=(SvLoginRect){f.modal.x+19,f.modal.y+215,88,36};
    f.connect=(SvLoginRect){f.modal.x+mw-117,f.modal.y+215,98,36};
    return f;
}
static inline int sv_login_contains(SvLoginRect r,float x,float y)
{ return x>=r.x && y>=r.y && x<r.x+r.w && y<r.y+r.h; }

#endif
