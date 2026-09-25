/* Real legacy handler, with external terminal/sound sinks observed by assertions. */
#include <assert.h>
#include <sys/time.h>
#include "../src/client/nclient.c"
static int drawn_max, drawn_cur, drawn_bar, drawn_boost, draws, warnings, messages, switches;
void prt_hp(int maximum, int current, bool bar, bool boosted) {
    drawn_max = maximum; drawn_cur = current; drawn_bar = bar; drawn_boost = boosted; ++draws;
}
int warning_page(void) { ++warnings; return 0; }
void c_msg_print(cptr message) { assert(strstr(message, "LOW HITPOINT")); ++messages; }
errr Term_switch(int screen) { assert(screen == 0); ++switches; return 0; }
errr Term_putstr(int x, int y, int n, byte a, char *text) {
    (void)x; (void)y; (void)n; (void)a; (void)text; return 0;
}
int main(void) {
    const unsigned char bytes[] = {24, 0, 100, 0, 10, 1};
    assert(Sockbuf_init(&rbuf, -1, 64, SOCKBUF_READ | SOCKBUF_LOCK) == 0);
    server_version = (version_type){4, 7, 0, 2, 0, 2, 0, 0};
    screen_hgt = 24; screen_icky = 1;
    c_cfg.hp_bar = TRUE; c_cfg.alert_hitpoint = TRUE; c_cfg.alert_offpanel_dam = TRUE;
    for (int split = 0; split < 6; ++split) {
        Sockbuf_clear(&rbuf); memcpy(rbuf.buf, bytes, sizeof(bytes)); rbuf.len = split;
        p_ptr->mhp = 500; p_ptr->chp = 400; p_ptr->window = 0; hp_boosted = TRUE;
        draws = warnings = messages = switches = 0;
        assert(Receive_hp() == 0);
        assert(p_ptr->mhp == 500 && p_ptr->chp == 400 && !p_ptr->window);
        assert(!draws && !warnings && !messages && !switches && hp_boosted);
        rbuf.len = 6; assert(Receive_hp() == 1);
        assert(drawn_max == 100 && drawn_cur == 10 && drawn_bar && !drawn_boost);
        assert(draws == 1 && warnings == 1 && messages == 1 && switches == 2);
        assert(p_ptr->mhp == 100 && p_ptr->chp == 10 && (p_ptr->window & PW_PLAYER));
    }
    /* Damage off-panel beeps; drain suppresses only that beep, not low-HP warning. */
    { unsigned char damage[] = {24, 0, 100, 0, 80, 0};
      Sockbuf_clear(&rbuf); memcpy(rbuf.buf, damage, 6); rbuf.len = 6; assert(Receive_hp() == 1);
      damage[4] = 5; warnings = messages = 0;
      Sockbuf_clear(&rbuf); memcpy(rbuf.buf, damage, 6); rbuf.len = 6; assert(Receive_hp() == 1);
      assert(warnings == 2 && messages == 1);
      damage[4] = 1; damage[5] = 255; warnings = messages = 0;
      Sockbuf_clear(&rbuf); memcpy(rbuf.buf, damage, 6); rbuf.len = 6; assert(Receive_hp() == 1);
      assert(warnings == 1 && messages == 1);
    }
    /* Marker normalization including boosted death and old wire layout. */
    { unsigned char boosted[] = {24, 0x27, 0x74, 0x27, 0x06};
      server_version.build = 1; Sockbuf_clear(&rbuf);
      memcpy(rbuf.buf, boosted, sizeof(boosted)); rbuf.len = sizeof(boosted);
      assert(Receive_hp() == 1);
      assert(drawn_max == 100 && drawn_cur == -10 && drawn_boost && hp_boosted);
    }
    /* The scanner's malformed-format failure contract; no invented invalid HP range. */
    { char value; Sockbuf_clear(&rbuf); assert(Packet_scanf(&rbuf, "%q", &value) == -1); }
    Sockbuf_cleanup(&rbuf);
    puts("legacy HP: fragmentation, value/markers, option, alerts and player invalidation passed");
    return 0;
}

int DgramRead(int fd, char *data, int size) { (void)fd; (void)data; (void)size; return -1; }
int DgramWrite(int fd, char *data, int size) { (void)fd; (void)data; (void)size; return -1; }
int GetSocketError(int fd) { (void)fd; return -1; }
