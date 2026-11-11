/* Include protocol/startup-prefix.h, add helper near existing identity_checks,
 * invoke once beside it. Two necessary untested framing paths, no rendering or
 * alternate behavior model. */
static void c011_prefix_boundary_checks(void)
{
    SvStartupPrefixConfig config={.version={4,4,3,0,0,5},.graphics=0,.char_transfer_bytes=4};
    const unsigned char old[]={PKT_LINE_INFO,0,1,'#',0x41,80};
    size_t consumed=99;
    assert(sv_startup_prefix_decode(&config,old,sizeof(old)-1,&consumed)==SV_WAITING && !consumed);
    assert(sv_startup_prefix_decode(&config,old,sizeof(old),&consumed)==SV_OK && consumed==sizeof(old));
    config=(SvStartupPrefixConfig){.version={4,9,4,0,0,0},.graphics=2,.char_transfer_bytes=4};
    /* One 80-column run: foreground u32, RLE marker, background u32/attr,
     * then true foreground attr and repeat. Coalesced following byte is retained. */
    const unsigned char dual[]={PKT_LINE_INFO,0,2,0,1,2,3,255,0,4,5,6,2,1,80,PKT_CHAR_INFO};
    consumed=99;
    assert(sv_startup_prefix_decode(&config,dual,sizeof(dual)-2,&consumed)==SV_WAITING && !consumed);
    assert(sv_startup_prefix_decode(&config,dual,sizeof(dual),&consumed)==SV_OK && consumed==sizeof(dual)-1);
    unsigned char malformed[sizeof(dual)];memcpy(malformed,dual,sizeof(dual));malformed[sizeof(dual)-2]=81;
    assert(sv_startup_prefix_decode(&config,malformed,sizeof(malformed),&consumed)==SV_DECODE_ERROR && !consumed);
}
