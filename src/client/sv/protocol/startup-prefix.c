#include "protocol/startup-prefix.h"
#define CLIENT_SIDE
#include "../../../common/defines.h"
#include "../../../common/pack.h"
#include <stdint.h>
#include <string.h>

static int version_compare(const int version[6], const int threshold[6])
{
    for (unsigned i=0;i<6;++i)
        if(version[i]!=threshold[i])return version[i]>threshold[i]?1:-1;
    return 0;
}
static uint32_t word(const unsigned char *p)
{ return (uint32_t)p[0]<<24 | (uint32_t)p[1]<<16 | (uint32_t)p[2]<<8 | p[3]; }
static SvResult strings(const unsigned char *bytes,size_t available,size_t *at,size_t count,size_t limit)
{
    for(size_t i=0;i<count;++i) {
        size_t n=available-*at;
        if(n>limit)n=limit;
        const unsigned char *end=memchr(bytes+*at,0,n);
        if(!end)return n==limit?SV_DECODE_ERROR:
            available==SV_STARTUP_PREFIX_LIMIT?SV_INPUT_OVERFLOW:SV_WAITING;
        *at=(size_t)(end-bytes)+1;
    }
    return SV_OK;
}
bool sv_startup_prefix_supports(unsigned char type)
{
    switch(type) {
    case PKT_PARTY: case PKT_GUILD: case PKT_GUILD_CFG:
    case PKT_WEATHERCOL: case PKT_PALETTE: case PKT_WEATHER:
    case PKT_SPELL_INFO: case PKT_POWERS_INFO:
    case PKT_SKILL_INIT: case PKT_SKILL_MOD: case PKT_SKILL_PTS:
    case PKT_TECHNIQUE_INFO: case PKT_BONI_COL: case PKT_INVENTORY_REV:
    case PKT_INDICATORS:
    case PKT_INVEN: case PKT_EQUIP:
    case PKT_SFX_VOLUME: case PKT_SFX_AMBIENT: case PKT_MUSIC:
    case PKT_UNIQUE_MONSTER:
    case PKT_INVEN_WIDE: case PKT_EQUIP_WIDE: case PKT_SOUND: case PKT_MUSIC_VOL:
    case PKT_LINE_INFO: case PKT_CHAR: case PKT_CHAR_DIRECT: return true;
    default:return false;
    }
}
SvResult sv_startup_prefix_decode(const SvStartupPrefixConfig *config,const void *data,
                                 size_t available,size_t *consumed)
{
    if(!consumed)return SV_INVALID;
    *consumed=0;
    if(!config || (available&&!data))return SV_INVALID;
    if(!available)return SV_WAITING;
    const unsigned char *bytes=data;
    if(!sv_startup_prefix_supports(bytes[0]))return SV_INVALID;
    /* Only one frame is inspected, even if the caller supplies a larger batch. */
    if(available>SV_STARTUP_PREFIX_LIMIT)available=SV_STARTUP_PREFIX_LIMIT;
    const int unicode[6]={4,8,1,0,0,0}, dual[6]={4,9,2,1,0,0};
    const int new_rle[6]={4,4,3,0,0,5}, weather_char[6]={4,7,4,6,0,0};
    unsigned width=version_compare(config->version,unicode)>=0?
        (config->char_transfer_bytes?config->char_transfer_bytes:1):1;
    if(width>4)width=4; /* Baseline switch default. */
    bool two_mask=config->graphics==2&&version_compare(config->version,dual)>=0;
    size_t at=1;
    SvResult result=SV_OK;
#define NEED(n) do { size_t need_=(n); if(need_>SV_STARTUP_PREFIX_LIMIT-at)return SV_INPUT_OVERFLOW; \
    if(need_>available-at)return available==SV_STARTUP_PREFIX_LIMIT?SV_INPUT_OVERFLOW:SV_WAITING; } while(0)
    switch(bytes[0]) {
    case PKT_SOUND: {
        const int spatial[6]={4,8,1,1,0,0}, identity[6]={4,4,5,3,0,0};
        const int kind[6]={4,4,5,1,0,0}, alternate[6]={4,4,5,0,0,0};
        size_t body=version_compare(config->version,spatial)>=0?28:
            version_compare(config->version,identity)>0?20:
            version_compare(config->version,kind)>0?12:
            version_compare(config->version,alternate)>0?8:1;
        NEED(body);at+=body;break;
    }
    case PKT_MUSIC_VOL: {
        const int wide[6]={4,9,2,1,0,1}, third[6]={4,8,1,2,0,0};
        size_t body=version_compare(config->version,wide)>=0?13:
            version_compare(config->version,third)>=0?4:3;
        NEED(body);at+=body;break;
    }
    case PKT_INVEN_WIDE: {
        const int artifact[6]={4,5,2,0,0,0}, wide[6]={4,7,0,0,0,0};
        const int identified[6]={4,7,1,1,0,0}, item_name[6]={4,4,4,2,0,0};
        size_t body=10+(version_compare(config->version,artifact)>0?2u:0u)+
            (version_compare(config->version,wide)>0?18u:9u);
        NEED(body);at+=body;
        result=strings(bytes,available,&at,1,
            version_compare(config->version,item_name)>0?ONAME_LEN:MAX_CHARS);
        if(result!=SV_OK)break;
        if(version_compare(config->version,identified)>0) { NEED(1);at++; }
        break;
    }
    case PKT_EQUIP_WIDE:
        NEED(13);at+=13;result=strings(bytes,available,&at,1,ONAME_LEN);
        if(result!=SV_OK)break;
        NEED(18);at+=18;break;
    case PKT_SFX_VOLUME: NEED(2);at+=2;break;
    case PKT_SFX_AMBIENT: NEED(4);at+=4;break;
    case PKT_MUSIC: {
        const int wide[6]={4,9,2,1,0,1}, third[6]={4,8,1,2,0,0};
        const int alternate[6]={4,5,6,0,0,1};
        size_t body=version_compare(config->version,wide)>=0?12:
            version_compare(config->version,third)>=0?3:
            version_compare(config->version,alternate)>0?2:1;
        NEED(body);at+=body;break;
    }
    case PKT_UNIQUE_MONSTER:
        NEED(8);at+=8;result=strings(bytes,available,&at,1,MAX_CHARS);break;
    case PKT_INVEN: case PKT_EQUIP: {
        /* Receive_inven/Receive_equip share the same versioned frame. %I is
         * a plain NUL-terminated item-name field capped by ONAME_LEN, not %s
         * (MAX_CHARS) or %S (MSG_LEN). No inventory application occurs here. */
        const int artifact[6]={4,5,2,0,0,0}, direction[6]={4,4,5,10,0,0};
        const int item_name[6]={4,4,4,2,0,0};
        size_t body=version_compare(config->version,artifact)>0?13:
            version_compare(config->version,direction)>0?11:10;
        NEED(body);at+=body;
        result=strings(bytes,available,&at,1,
            version_compare(config->version,item_name)>0?ONAME_LEN:MAX_CHARS);
        break;
    }
    case PKT_PARTY: case PKT_GUILD:
        result=strings(bytes,available,&at,3,MAX_CHARS);break;
    case PKT_GUILD_CFG: {
        NEED(28);
        int32_t count=(int32_t)word(bytes+13);
        at=29;
        /* Legacy loops over all transmitted adders, storing only the first 5.
         * A negative count has no trailing strings in that baseline loop. */
        if(count>0) {
            if((uint32_t)count>SV_STARTUP_PREFIX_LIMIT-at)return SV_INPUT_OVERFLOW;
            result=strings(bytes,available,&at,(size_t)count,MAX_CHARS);
        }
        break;
    }
    case PKT_WEATHERCOL:
        NEED(version_compare(config->version,weather_char)>=0?4u:2u);
        at+=version_compare(config->version,weather_char)>=0?4u:2u;break;
    case PKT_PALETTE: NEED(4);at+=4;break;
    case PKT_POWERS_INFO: NEED(16);at+=16;break;
    case PKT_SKILL_INIT:
        NEED(13);at+=13;result=strings(bytes,available,&at,3,MSG_LEN);break;
    case PKT_SKILL_PTS: case PKT_INVENTORY_REV: case PKT_INDICATORS:
        NEED(4);at+=4;break;
    case PKT_TECHNIQUE_INFO: NEED(8);at+=8;break;
    case PKT_SKILL_MOD: {
        const int old[6]={4,4,1,2,0,0}, middle[6]={4,4,1,7,0,0};
        const int compact[6]={4,4,4,1,0,0};
        size_t body=version_compare(config->version,old)<=0?20:
            version_compare(config->version,middle)<=0?24:
            version_compare(config->version,compact)<=0?28:21;
        NEED(body);at+=body;break;
    }
    case PKT_BONI_COL: {
        const int extended[6]={4,6,1,2,0,0}, sigil[6]={4,5,9,0,0,0};
        size_t body=version_compare(config->version,unicode)>=0?43:
            version_compare(config->version,extended)>0?40:
            version_compare(config->version,sigil)>0?37:35;
        NEED(body);at+=body;break;
    }
    case PKT_SPELL_INFO:
        NEED(18);at+=18;result=strings(bytes,available,&at,1,MAX_CHARS);break;
    case PKT_WEATHER: {
        NEED(32);
        int32_t clouds=(int32_t)word(bytes+29);
        at=33;
        size_t count=clouds==-1?0:clouds<0?10:(size_t)clouds;
        if(count>(SV_STARTUP_PREFIX_LIMIT-at)/32)return SV_INPUT_OVERFLOW;
        NEED(count*32);at+=count*32;break;
    }
    case PKT_CHAR: case PKT_CHAR_DIRECT:
        NEED(2+(width+1)*(two_mask?2u:1u));
        at+=2+(width+1)*(two_mask?2u:1u);break;
    case PKT_LINE_INFO: {
        NEED(2);at+=2; /* signed row, no viewport-derived length */
        unsigned columns=0;
        while(columns<80) {
            NEED((width+1)*(two_mask?2u:1u));
            unsigned attribute=bytes[at+width];
            at+=(width+1)*(two_mask?2u:1u);
            unsigned repeat=1;
            if(version_compare(config->version,new_rle)>0) {
                if(attribute==TERM_RESERVED_RLE) { NEED(2);repeat=bytes[at+1];at+=2; }
            } else if(attribute&0x40) { NEED(1);repeat=bytes[at++]; }
            if(!repeat || repeat>80-columns)return SV_DECODE_ERROR;
            columns+=repeat;
        }
        break;
    }
    default:return SV_INVALID;
    }
#undef NEED
    if(result!=SV_OK)return result;
    *consumed=at;
    return SV_OK;
}
