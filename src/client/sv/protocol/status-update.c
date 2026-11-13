#include "protocol/status-update.h"
#define CLIENT_SIDE
#include "../../../common/defines.h"
#include "../../../common/pack.h"
#include <string.h>
/* Cursor remains local until the complete packet has been checked. */
typedef struct { const unsigned char *p; size_t size, at; SvResult result; } Cursor;
static unsigned byte(Cursor *c)
{
    if (c->result != SV_OK) return 0;
    if (c->at >= c->size) { c->result = SV_WAITING; return 0; }
    return c->p[c->at++];
}
static int word(Cursor *c) { unsigned a=byte(c),b=byte(c); return (int16_t)(a<<8|b); }
static int32_t number(Cursor *c)
{ uint32_t a=byte(c),b=byte(c),d=byte(c),e=byte(c); return (int32_t)(a<<24|b<<16|d<<8|e); }
static void text(Cursor *c, char out[SV_STATUS_TEXT])
{
    if (c->result != SV_OK) return;
    size_t available=c->size-c->at;
    if(available>MAX_CHARS)available=MAX_CHARS;
    const unsigned char *end=memchr(c->p+c->at,0,available);
    if(!end){c->result=available==MAX_CHARS?SV_DECODE_ERROR:SV_WAITING;return;}
    size_t length=(size_t)(end-(c->p+c->at));
    memcpy(out,c->p+c->at,length);out[length]=0;c->at+=length+1;
}
static int compare(const int version[6], int a,int b,int c,int d,int e,int f)
{ int threshold[6]={a,b,c,d,e,f};for(int i=0;i<6;++i)if(version[i]!=threshold[i])return version[i]>threshold[i]?1:-1;return 0; }
#define CMP(a,b,c,d,e,f) compare(version,a,b,c,d,e,f)
bool sv_status_packet_supported(unsigned char type)
{
    switch(type){
    case PKT_MP:case PKT_STAMINA:case PKT_SANITY:case PKT_STAT:case PKT_AC:
    case PKT_VARIOUS:case PKT_PLUSSES:case PKT_EXPERIENCE:case PKT_GOLD:
    case PKT_CONFUSED:case PKT_POISON:case PKT_STUDY:case PKT_BPR:case PKT_FOOD:
    case PKT_FEAR:case PKT_SPEED:case PKT_CUT:case PKT_BLIND:case PKT_STUN:
    case PKT_MONSTER_HEALTH:case PKT_AFK:case PKT_ENCUMBERMENT:case PKT_EXTRA_STATUS:
    case PKT_MARTYR:case PKT_IDLE:case PKT_INDICATORS:case PKT_DEPTH:return true;
    default:return false;
    }
}
SvResult sv_status_decode(const int version[6],const void *bytes,size_t size,SvHudUpdate *out,size_t *consumed)
{
    if(!consumed)return SV_INVALID;
    *consumed=0;
    if(!version||!out||(size&&!bytes))return SV_INVALID;
    if(!size)return SV_WAITING;
    Cursor c={bytes,size,0,SV_OK};unsigned type=byte(&c);
    SvHudUpdate next={0};SvHud *h=&next.values;
    switch(type){
    case PKT_MP:case PKT_STAMINA:{
        next.field=type==PKT_MP?SV_HUD_MANA:SV_HUD_STAMINA;
        SvHudResource *r=type==PKT_MP?&h->mana:&h->stamina;
        r->maximum=word(&c);r->current=word(&c);r->bar=r->maximum>10000;
        if(r->bar)r->maximum-=10000;
        break;
    }
    case PKT_SANITY:
        next.field=SV_HUD_SANITY;h->sanity.attribute=byte(&c);text(&c,h->sanity.label);
        h->sanity.damage_available=CMP(4,6,1,2,0,0)>0;
        h->sanity.numeric_available=CMP(4,8,1,3,0,0)>=0;
        if(h->sanity.damage_available)h->sanity.damage=byte(&c);
        if(h->sanity.numeric_available){h->sanity.current=word(&c);h->sanity.maximum=word(&c);}
        break;
    case PKT_STAT:{
        next.field=SV_HUD_STATS;unsigned index=byte(&c);SvHudStat stat={0};
        stat.maximum=word(&c);stat.current=word(&c);stat.index=word(&c);stat.base=word(&c);
        if(CMP(4,7,4,6,0,0)>=0)stat.temporary=word(&c);
        stat.boosted=stat.temporary!=0;
        if(!stat.temporary && (index&0x10)){stat.boosted=true;index&=~0x10u;}
        if(c.result==SV_OK&&index>=6)return SV_DECODE_ERROR;
        if(index<6){stat.available=true;h->stats[index]=stat;next.stat_index=(int)index;}
        break;
    }
    case PKT_AC:
        next.field=SV_HUD_ARMOUR;h->armour.base=word(&c);h->armour.bonus=word(&c);
        if(CMP(4,7,3,1,0,0)>=0){
            h->armour.unknown=h->armour.base>15000;if(h->armour.unknown)h->armour.base-=20000;
            h->armour.boosted=h->armour.base>5000;if(h->armour.boosted)h->armour.base-=10000;
        }else if(CMP(4,7,3,0,0,0)>=0){h->armour.boosted=(h->armour.base&0x1000)!=0;h->armour.base&=~0x1000;}
        break;
    case PKT_VARIOUS:
        next.field=SV_HUD_DEMOGRAPHICS;h->demographics.height=word(&c);h->demographics.weight=word(&c);
        h->demographics.age=word(&c);h->demographics.social_class=word(&c);text(&c,h->demographics.body);break;
    case PKT_PLUSSES:
        next.field=SV_HUD_COMBAT;h->combat.hit=word(&c);h->combat.damage=word(&c);
        h->combat.ranged_hit=word(&c);h->combat.ranged_damage=word(&c);
        h->combat.melee_hit=word(&c);h->combat.melee_damage=word(&c);break;
    case PKT_EXPERIENCE:
        next.field=SV_HUD_EXPERIENCE;h->experience.level=word(&c);h->experience.maximum_level=word(&c);
        h->experience.maximum_player_level=word(&c);h->experience.maximum=number(&c);h->experience.current=number(&c);
        h->experience.advance=number(&c);h->experience.previous=CMP(4,5,6,0,0,1)>0?number(&c):h->experience.advance;
        h->experience.fractional=h->experience.level>=1000;if(h->experience.fractional)h->experience.level-=1000;break;
    case PKT_GOLD:next.field=SV_HUD_CURRENCY;h->currency.carried=number(&c);h->currency.bank=number(&c);break;
    case PKT_CONFUSED:next.field=SV_HUD_CONFUSION;h->confusion=byte(&c);break;
    case PKT_POISON:next.field=SV_HUD_POISON;h->poison=byte(&c);break;
    case PKT_STUDY:next.field=SV_HUD_STUDY;h->study=byte(&c);break;
    case PKT_BPR:next.field=SV_HUD_BLOWS;h->blows.value=byte(&c);h->blows.attribute=byte(&c);
        if(CMP(4,9,1,0,0,1)>=0)text(&c,h->blows.label);break;
    case PKT_FOOD:next.field=SV_HUD_FOOD;h->food=(uint16_t)word(&c);break;
    case PKT_FEAR:next.field=SV_HUD_FEAR;h->fear=byte(&c);break;
    case PKT_SPEED:next.field=SV_HUD_SPEED;h->speed=word(&c);
        if(h->speed>=0){h->speed_boosted=(h->speed&0x100)!=0;h->speed&=~0x100;}break;
    case PKT_CUT:next.field=SV_HUD_CUT;h->cut=word(&c);break;
    case PKT_BLIND:next.field=SV_HUD_BLIND;h->blind=byte(&c);break;
    case PKT_STUN:next.field=SV_HUD_STUN;h->stun=word(&c);break;
    case PKT_MONSTER_HEALTH:next.field=SV_HUD_MONSTER;h->monster.health=byte(&c);h->monster.attribute=byte(&c);break;
    case PKT_AFK:next.field=SV_HUD_AFK;h->afk=byte(&c);break;
    case PKT_MARTYR:next.field=SV_HUD_MARTYR;h->martyr=(int8_t)byte(&c);break;
    case PKT_IDLE:next.field=SV_HUD_IDLE;h->idle=byte(&c);break;
    case PKT_ENCUMBERMENT:{
        next.field=SV_HUD_ENCUMBRANCE;
        /* Wire order puts heavy_tool last, while public semantic order retains it there. */
        unsigned count=CMP(4,8,1,0,0,0)>=0?15:CMP(4,4,2,0,0,0)>0?13:12;
        for(unsigned i=0;i<count;++i){unsigned target=i;
            if(count==12&&i>=11)target=i+1;
            h->encumbrance[target]=byte(&c);
        }break;
    }
    case PKT_EXTRA_STATUS:next.field=SV_HUD_EXTRA;text(&c,h->extra);break;
    case PKT_INDICATORS:next.field=SV_HUD_INDICATORS;h->indicators=(uint32_t)number(&c);break;
    case PKT_DEPTH:{
        next.field=SV_HUD_DEPTH;h->depth.x=word(&c);h->depth.y=word(&c);h->depth.z=word(&c);h->depth.town=byte(&c);
        if(CMP(4,4,1,6,0,0)>0){h->depth.colour=byte(&c);h->depth.sector_colour=byte(&c);}
        else {int old_colour=word(&c);h->depth.colour=(unsigned char)old_colour;
            if(CMP(4,4,1,5,0,0)<=0)h->depth.colour=old_colour?TERM_ORANGE:TERM_WHITE;
            h->depth.sector_colour=TERM_L_GREEN;}
        text(&c,h->depth.location);
        if(CMP(4,5,9,0,0,0)>0)text(&c,h->depth.location_secondary);
        if(CMP(4,6,1,2,0,0)>0)text(&c,h->depth.preposition);
        else strcpy(h->depth.preposition,(CMP(4,5,9,0,0,0)>0?h->depth.location_secondary[0]:h->depth.location[0])?"in":"of");
        h->depth.no_tele=h->depth.sector_colour==TERM_L_DARK;break;
    }
    default:return SV_DECODE_ERROR;
    }
    if(c.result!=SV_OK)return c.result;
    *out=next;*consumed=c.at;return SV_OK;
}
