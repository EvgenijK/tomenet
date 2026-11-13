#ifndef SV_PROTOCOL_VISUALS_H
#define SV_PROTOCOL_VISUALS_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
/* SV-owned semantic definitions, preserving Client_setup array limits. */
typedef struct { uint32_t character; unsigned char attribute; } SvVisualDefinition;
typedef struct {
    SvVisualDefinition unknown[127], features[512], kinds[1280], monsters[1280];
} SvVisualDefinitions;
static inline bool sv_visual_atleast(const int v[6],const int t[6])
{
    for (int i=0;i<6;++i) if (v[i]!=t[i]) return v[i]>t[i];
    return true;
}
/* PLAY uses historical table lengths; live Send_client_setup uses its own
 * inclusive feature gate and full kind/monster tables. Caller reserves 16 KiB. */
static inline size_t sv_visual_encode(const SvVisualDefinitions *d,const int v[6],
                                      bool play,unsigned char *out)
{
    const int wide[6]={4,8,1,0,0,0}, expanded[6]={4,6,1,2,0,1};
    const int play_features[6]={4,9,1,2,0,3}, live_features[6]={4,9,2,1,0,3};
    size_t counts[4]={127,sv_visual_atleast(v,play?play_features:live_features)?512:256,
        !play || sv_visual_atleast(v,expanded)?1280:1024,
        !play || sv_visual_atleast(v,expanded)?1280:1152};
    const SvVisualDefinition *tables[4]={d->unknown,d->features,d->kinds,d->monsters};
    unsigned char *at=out;
    for (unsigned table=0;table<4;++table) for (size_t i=0;i<counts[table];++i) {
        const SvVisualDefinition *entry=&tables[table][i];
        *at++=entry->attribute;
        if (sv_visual_atleast(v,wide)) {
            *at++=(unsigned char)(entry->character>>24);
            *at++=(unsigned char)(entry->character>>16);
            *at++=(unsigned char)(entry->character>>8);
        }
        *at++=(unsigned char)entry->character;
    }
    return (size_t)(at-out);
}
#endif
