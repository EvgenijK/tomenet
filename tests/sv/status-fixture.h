#ifndef SV_TEST_STATUS_FIXTURE_H
#define SV_TEST_STATUS_FIXTURE_H
#include "app.h"
/* Independently specified baseline wire fixtures. No test-side decoding or
 * semantic update logic: every byte goes through production app/protocol. */
static const int sv_status_fixture_version[6] = {4,9,3,0,2,0};
typedef struct { const char *name; const unsigned char *bytes; size_t size; } SvStatusFixture;
static const unsigned char sv_status_bytes_hp[] = {24,39,116,39,91,1};
static const unsigned char sv_status_bytes_mana[] = {25,39,66,0,30};
static const unsigned char sv_status_bytes_stamina[] = {172,39,96,0,60};
static const unsigned char sv_status_bytes_sanity[] = {130,13,83,111,117,110,100,0,1,0,40,0,60};
static const unsigned char sv_status_bytes_stats[] = {28,0,0,118,0,108,0,20,0,110,0,3};
static const unsigned char sv_status_bytes_armour[] = {21,117,171,0,7};
static const unsigned char sv_status_bytes_demographics[] = {27,0,180,0,75,0,42,0,9,80,108,97,121,101,114,0};
static const unsigned char sv_status_bytes_combat[] = {20,0,1,0,2,0,3,0,4,0,5,0,6};
static const unsigned char sv_status_bytes_experience[] = {22,3,252,0,25,0,23,0,1,226,64,0,1,212,192,0,1,251,208,0,1,173,176};
static const unsigned char sv_status_bytes_currency[] = {23,0,0,4,210,0,0,22,46};
static const unsigned char sv_status_bytes_confusion[] = {37,1};
static const unsigned char sv_status_bytes_poison[] = {39,3};
static const unsigned char sv_status_bytes_study[] = {43,4};
static const unsigned char sv_status_bytes_blows[] = {129,3,13,51,32,66,112,82,0};
static const unsigned char sv_status_bytes_food[] = {35,19,136};
static const unsigned char sv_status_bytes_fear[] = {38,1};
static const unsigned char sv_status_bytes_speed[] = {42,1,10};
static const unsigned char sv_status_bytes_cut[] = {44,0,75};
static const unsigned char sv_status_bytes_blind[] = {36,3};
static const unsigned char sv_status_bytes_stun[] = {45,0,52};
static const unsigned char sv_status_bytes_monster[] = {59,6,12};
static const unsigned char sv_status_bytes_afk[] = {163,1};
static const unsigned char sv_status_bytes_encumbrance[] = {164,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
static const unsigned char sv_status_bytes_extra[] = {174,255,114,88,128,0};
static const unsigned char sv_status_bytes_martyr[] = {136,1};
static const unsigned char sv_status_bytes_idle[] = {138,1};
static const unsigned char sv_status_bytes_indicators[] = {197,128,0,0,17};
static const SvStatusFixture sv_status_fixtures[] = {
    {"hp", sv_status_bytes_hp, sizeof(sv_status_bytes_hp)},
    {"mana", sv_status_bytes_mana, sizeof(sv_status_bytes_mana)},
    {"stamina", sv_status_bytes_stamina, sizeof(sv_status_bytes_stamina)},
    {"sanity", sv_status_bytes_sanity, sizeof(sv_status_bytes_sanity)},
    {"stats", sv_status_bytes_stats, sizeof(sv_status_bytes_stats)},
    {"armour", sv_status_bytes_armour, sizeof(sv_status_bytes_armour)},
    {"demographics", sv_status_bytes_demographics, sizeof(sv_status_bytes_demographics)},
    {"combat", sv_status_bytes_combat, sizeof(sv_status_bytes_combat)},
    {"experience", sv_status_bytes_experience, sizeof(sv_status_bytes_experience)},
    {"currency", sv_status_bytes_currency, sizeof(sv_status_bytes_currency)},
    {"confusion", sv_status_bytes_confusion, sizeof(sv_status_bytes_confusion)},
    {"poison", sv_status_bytes_poison, sizeof(sv_status_bytes_poison)},
    {"study", sv_status_bytes_study, sizeof(sv_status_bytes_study)},
    {"blows", sv_status_bytes_blows, sizeof(sv_status_bytes_blows)},
    {"food", sv_status_bytes_food, sizeof(sv_status_bytes_food)},
    {"fear", sv_status_bytes_fear, sizeof(sv_status_bytes_fear)},
    {"speed", sv_status_bytes_speed, sizeof(sv_status_bytes_speed)},
    {"cut", sv_status_bytes_cut, sizeof(sv_status_bytes_cut)},
    {"blind", sv_status_bytes_blind, sizeof(sv_status_bytes_blind)},
    {"stun", sv_status_bytes_stun, sizeof(sv_status_bytes_stun)},
    {"monster", sv_status_bytes_monster, sizeof(sv_status_bytes_monster)},
    {"afk", sv_status_bytes_afk, sizeof(sv_status_bytes_afk)},
    {"encumbrance", sv_status_bytes_encumbrance, sizeof(sv_status_bytes_encumbrance)},
    {"extra", sv_status_bytes_extra, sizeof(sv_status_bytes_extra)},
    {"martyr", sv_status_bytes_martyr, sizeof(sv_status_bytes_martyr)},
    {"idle", sv_status_bytes_idle, sizeof(sv_status_bytes_idle)},
    {"indicators", sv_status_bytes_indicators, sizeof(sv_status_bytes_indicators)},
};
static inline SvResult sv_status_fixture_feed(SvApp *app)
{
    for (size_t i=0;i<sizeof(sv_status_fixtures)/sizeof(sv_status_fixtures[0]);++i) {
        SvResult result=sv_app_receive(app,sv_app_view(app).generation,
            sv_status_fixtures[i].bytes,sv_status_fixtures[i].size);
        if (result!=SV_OK) return result;
        SvStep step=sv_app_step(app,1);
        if (step.result!=SV_OK || step.processed!=1) return step.result==SV_OK?SV_INVALID:step.result;
    }
    return SV_OK;
}

/* Current HTML default active icon states, over the independent all-field
 * fixture. This is wire data only: the production decoder/model still owns
 * interpretation. IDLE deliberately remains one to disprove IDLE -> Resting. */
static inline SvResult sv_status_fixture_matched_icons(SvApp *app)
{
    static const unsigned char packets[][16]={
        {164,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
        {197,0,0,0,0x5f},{37,0},{39,0},{38,0},{44,0,0},
        {36,0},{45,0,0},{35,0x13,0x88},{138,1}
    };
    static const size_t sizes[]={16,5,2,2,2,3,2,3,3,2};
    for(size_t i=0;i<sizeof(sizes)/sizeof(sizes[0]);++i){
        SvResult result=sv_app_receive(app,sv_app_view(app).generation,packets[i],sizes[i]);
        if(result!=SV_OK)return result;
        SvStep step=sv_app_step(app,1);
        if(step.result!=SV_OK||step.processed!=1)return step.result==SV_OK?SV_INVALID:step.result;
    }
    return SV_OK;
}


/* Current HTML values that the status protocol can actually supply. This
 * optional visual scenario overlays raw packets, never presentation fields.
 * Missing identity/skills stay unavailable; tracked health is quantized by the
 * baseline wire format (7/10 versus HTML 72%). True XP maximum equals current
 * when not drained; HTML Character calls the next threshold "Max Exp". */
static inline SvResult sv_status_fixture_html(SvApp *app)
{
    static const unsigned char packets[][32]={
        /* hp 1912/1912, bar, unboosted */
        {24,46,136,7,120,0},
        /* mana 275/275, bar */
        {25,40,35,1,19},
        /* stamina 10/10, bar */
        {172,39,26,0,10},
        /* sanity Sound, light green, 100/100 */
        {130,13,83,111,117,110,100,0,0,0,100,0,100},
        /* stat 0, current = max, unboosted */
        {28,0,0,198,0,198,0,33,0,198,0,0},
        /* stat 1, current = max, unboosted */
        {28,1,0,198,0,198,0,33,0,198,0,0},
        /* stat 2, current = max, unboosted */
        {28,2,0,178,0,178,0,31,0,178,0,0},
        /* stat 3, current = max, unboosted */
        {28,3,0,158,0,158,0,29,0,158,0,0},
        /* stat 4, current = max, unboosted */
        {28,4,0,198,0,198,0,33,0,198,0,0},
        /* stat 5, current = max, unboosted */
        {28,5,0,108,0,108,0,24,0,108,0,0},
        /* armour 193, no boost/unknown */
        {21,0,193,0,0},
        /* body Vampire, demographics from HTML */
        {27,0,72,0,146,0,124,0,57,86,97,109,112,105,114,101,0},
        /* combat melee +47/+38; ranged +31/+12 */
        {20,0,0,0,0,0,31,0,12,0,47,0,38},
        /* experience level57, 68% of16800000..19200000, no drain */
        {22,0,57,0,57,0,57,1,25,64,0,1,25,64,0,1,36,248,0,1,0,89,0},
        /* gold952358, bank0 */
        {23,0,14,136,38,0,0,0,0},
        /* blows4, light green */
        {129,4,13,52,32,66,112,82,0},
        /* speed+41, unboosted */
        {42,0,41},
        /* tracked health7/10 (wire cannot encode HTML72%) */
        {59,7,4},
        /* AFK off */
        {163,0},
        /* study off */
        {43,0},
        /* extra status clear */
        {174,0},
        /* martyr off */
        {136,0},
    };
    static const size_t sizes[]={6,5,5,13,12,12,12,12,12,12,5,17,13,23,9,9,3,3,2,2,2,2};
    for(size_t i=0;i<sizeof(sizes)/sizeof(sizes[0]);++i){
        SvResult result=sv_app_receive(app,sv_app_view(app).generation,packets[i],sizes[i]);
        if(result!=SV_OK)return result;
        SvStep step=sv_app_step(app,1);
        if(step.result!=SV_OK||step.processed!=1)return step.result==SV_OK?SV_INVALID:step.result;
    }
    return SV_OK;
}

/* Server /snbar label variants at full sanity, with the same numeric wire
 * payload. This diagnostic overlay exercises the production packet path; it
 * neither grants the server command nor changes the default baseline corpus. */
static inline SvResult sv_status_fixture_sanity_mode(SvApp *app,const char *mode)
{
    const char *label=!strcmp(mode,"word")?"    Sound":!strcmp(mode,"bar")?"*********":!strcmp(mode,"percent")?"     100%":!strcmp(mode,"exact")?" 100/ 100":NULL;
    if(!label)return SV_INVALID;
    unsigned char packet[32]={130,13};size_t n=2;
    while(*label)packet[n++]=(unsigned char)*label++;
    packet[n++]=0;packet[n++]=0;packet[n++]=0;packet[n++]=100;packet[n++]=0;packet[n++]=100;
    SvResult result=sv_app_receive(app,sv_app_view(app).generation,packet,n);
    if(result!=SV_OK)return result;SvStep step=sv_app_step(app,1);
    return step.result!=SV_OK?step.result:step.processed==1?SV_OK:SV_INVALID;
}

#endif
