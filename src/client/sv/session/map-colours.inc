/* SV-local adaptation of z-term.c anim2static/flick_colour. Preserve source
 * categories/probabilities; RNG/clock/scopes are explicit and drawing is pure. */
enum {
    TERM_ACID=20,
    TERM_ANIM_LAVA_EAST=91,
    TERM_ANIM_LAVA_NORTH=93,
    TERM_ANIM_LAVA_SOUTH=94,
    TERM_ANIM_LAVA_WEST=92,
    TERM_ANIM_WATER_EAST=87,
    TERM_ANIM_WATER_NORTH=89,
    TERM_ANIM_WATER_SOUTH=90,
    TERM_ANIM_WATER_WEST=88,
    TERM_ANNI=30,
    TERM_BLUE=6,
    TERM_BNW=120,
    TERM_BNWKS=123,
    TERM_BNWKS2=124,
    TERM_BNWM=121,
    TERM_BNWSR=122,
    TERM_COLD=19,
    TERM_CONF=22,
    TERM_CURSE=29,
    TERM_DARK=0,
    TERM_DARKNESS=26,
    TERM_DETO=46,
    TERM_DISE=35,
    TERM_DISI=42,
    TERM_ELEC=21,
    TERM_EMBER=55,
    TERM_FIRE=18,
    TERM_FIRETHIN=56,
    TERM_FORC=37,
    TERM_GRAV=38,
    TERM_GREEN=5,
    TERM_HALF=31,
    TERM_HAVOC=58,
    TERM_HELLFIRE=51,
    TERM_HOLYFIRE=50,
    TERM_HOLYORB=49,
    TERM_ICE=44,
    TERM_INER=36,
    TERM_LAMP=53,
    TERM_LAMP_DARK=54,
    TERM_LITE=25,
    TERM_L_BLUE=14,
    TERM_L_DARK=8,
    TERM_L_GREEN=13,
    TERM_L_RED=12,
    TERM_L_UMBER=15,
    TERM_L_WHITE=9,
    TERM_MANA=41,
    TERM_METEOR=40,
    TERM_MULTI=16,
    TERM_NETH=34,
    TERM_NEXU=33,
    TERM_NUKE=47,
    TERM_OLD3_BNW=248,
    TERM_OLD3_BNWKS=251,
    TERM_OLD3_BNWKS2=252,
    TERM_OLD3_BNWM=249,
    TERM_OLD3_BNWSR=250,
    TERM_OLD3_PVP=254,
    TERM_OLD3_PVPBB=253,
    TERM_OLD_BNW=32,
    TERM_OLD_PVP=64,
    TERM_ORANGE=3,
    TERM_PLAS=45,
    TERM_POIS=17,
    TERM_PSI=32,
    TERM_PVP=126,
    TERM_PVPBB=125,
    TERM_RED=4,
    TERM_SELECTOR=59,
    TERM_SEL_BLUE=62,
    TERM_SEL_GREEN=95,
    TERM_SEL_RED=61,
    TERM_SHAR=24,
    TERM_SHIELDI=28,
    TERM_SHIELDM=27,
    TERM_SLATE=2,
    TERM_SMOOTHPAL=60,
    TERM_SOUN=23,
    TERM_SRCLITE=63,
    TERM_STARLITE=57,
    TERM_THUNDER=52,
    TERM_TIME=39,
    TERM_UMBER=7,
    TERM_UNBREATH=48,
    TERM_VIOLET=10,
    TERM_WATE=43,
    TERM_WHITE=1,
    TERM_YELLOW=11
};
static unsigned char animated_colour(ColourContext *,unsigned char);
static unsigned char static_colour(ColourContext *c,unsigned char attr) {
 switch (attr) {
 case TERM_BNW:
 case TERM_BNWM:
 case TERM_BNWSR:
 case TERM_BNWKS:
 case TERM_BNWKS2:
 case TERM_PVPBB:
 case TERM_PVP:
  return(TERM_WHITE);
 case TERM_SHIELDM: return(TERM_L_RED);
 case TERM_SHIELDI: return(TERM_VIOLET);
 case TERM_MULTI:
  return(TERM_VIOLET);
 case TERM_FIRE: return(TERM_RED);
 case TERM_POIS: return(TERM_L_GREEN);
 case TERM_COLD: return(TERM_L_WHITE);
 case TERM_ELEC: return(TERM_L_BLUE);
 case TERM_HALF:
  return(static_colour(c,TERM_SMOOTHPAL));
 case TERM_ACID: return(TERM_SLATE);
 case TERM_CONF: return(TERM_L_UMBER);
 case TERM_SOUN: return(TERM_YELLOW);
 case TERM_SHAR: return(TERM_UMBER);
 case TERM_LITE: return(TERM_YELLOW);
 case TERM_DARKNESS: return(TERM_L_DARK);
 case TERM_CURSE:
  return(TERM_L_DARK);
 case TERM_ANNI:
  return(TERM_L_DARK);
 case TERM_PSI:
  return(TERM_YELLOW);
 case TERM_NEXU:
  return(TERM_VIOLET);
 case TERM_NETH:
  return(TERM_L_DARK);
 case TERM_DISE:
  return(TERM_ORANGE);
 case TERM_INER:
  return(TERM_SLATE);
 case TERM_FORC:
  return(TERM_L_WHITE);
 case TERM_GRAV:
  return(TERM_SLATE);
 case TERM_TIME:
  return(TERM_L_GREEN);
 case TERM_METEOR:
  return(TERM_UMBER);
 case TERM_MANA:
  return(TERM_VIOLET);
 case TERM_WATE:
  return(TERM_BLUE);
 case TERM_ICE:
  return(TERM_WHITE);
 case TERM_PLAS:
  return(TERM_L_RED);
 case TERM_DETO:
  return(TERM_L_RED);
 case TERM_DISI:
  return(TERM_L_DARK);
 case TERM_NUKE:
  return(TERM_GREEN);
 case TERM_UNBREATH:
  return(TERM_GREEN);
 case TERM_HOLYORB:
  return(TERM_ORANGE);
 case TERM_HOLYFIRE:
  return(TERM_ORANGE);
 case TERM_HELLFIRE:
  return(TERM_RED);
 case TERM_THUNDER:
  return(TERM_L_BLUE);
 case TERM_LAMP:
  return(TERM_YELLOW);
 case TERM_LAMP_DARK:
  return(TERM_L_UMBER);
 case TERM_EMBER:
  return(TERM_RED);
 case TERM_STARLITE:
  return(TERM_WHITE);
 case TERM_HAVOC:
  return(TERM_L_RED);
 case TERM_FIRETHIN:
  return(TERM_RED);
 case TERM_SELECTOR:
  switch ((unsigned)((int64_t)(c->e->now_ms/100)) % 14) {
  case 0: case 1:
  case 14: case 15:
   return(TERM_UMBER);
  case 2: case 3:
  case 12: case 13:
   return(TERM_ORANGE);
  case 4: case 5: case 6:
  case 7: case 8:
  case 9: case 10: case 11:
   return(TERM_YELLOW);
  }
  return(TERM_VIOLET);
 case TERM_SMOOTHPAL:
  switch ((((unsigned)((int64_t)(c->e->now_ms/100)) * 10) / 52) % 6) {
  case 0: return(TERM_L_RED);
  case 1: return(TERM_ORANGE);
  case 2: return(TERM_YELLOW);
  case 3: return(TERM_L_GREEN);
  case 4: return(TERM_L_BLUE);
  case 5: return(TERM_VIOLET);
  }
  return(TERM_VIOLET);
 case TERM_SEL_RED:
  switch ((unsigned)((int64_t)(c->e->now_ms/100)) % 5) {
  case 0:
   return(TERM_L_RED);
  case 1: case 2:
   return(TERM_RED);
  case 3: case 4:
   return(TERM_L_DARK);
  }
  return(TERM_VIOLET);
 case TERM_SEL_BLUE:
  if (true
      ) {
   switch ((unsigned)((int64_t)(c->e->now_ms/100)) % 14) {
   case 0: case 1:
   case 14: case 15:
    return(TERM_BLUE);
   case 2: case 3:
   case 12: case 13:
    return(TERM_SLATE);
   case 7: case 8:
   case 4: case 5: case 6:
   case 9: case 10: case 11:
    return(TERM_L_BLUE);
   }
  } else {
   return(TERM_L_BLUE);
  }
  return(TERM_VIOLET);
 case TERM_SRCLITE: return(searchlight(c));
 case TERM_SEL_GREEN:
  switch ((unsigned)((int64_t)(c->e->now_ms/100)) % 14) {
  case 0: case 1:
  case 14: case 15:
   return(TERM_L_DARK);
  case 2: case 3:
  case 12: case 13:
   return(TERM_GREEN);
  case 4: case 5: case 6:
  case 7: case 8:
  case 9: case 10: case 11:
   return(TERM_L_GREEN);
  }
  return(TERM_VIOLET);
 default:
  return(TERM_L_WHITE);
 }
}
static unsigned char animated_colour(ColourContext *c,unsigned char attr) {
 unsigned char flags = attr;
 if (c->e->options.no_flicker ||
     (!c->e->options.subterm_flicker && c->scope != SV_EFFECT_MAP) ||
     (c->e->options.misc_no_flicker && c->scope != SV_EFFECT_MAP && c->scope != SV_EFFECT_MESSAGE)
     ) {
  return(static_colour(c,attr));
 }
 if (!is_newer_than(c->e->version, 4, 5, 1, 1, 0, 0)) {
  if (flags & TERM_OLD_PVP) {
   flags &= ~TERM_OLD_PVP;
   flags |= TERM_PVP;
  }
  if (flags & TERM_OLD_BNW) {
   flags &= ~TERM_OLD_BNW;
   flags |= TERM_BNW;
  }
 }
 if (!is_newer_than(c->e->version, 4, 7, 1, 1, 0, 0)) {
  if (attr & TERM_PVP) attr = TERM_PVP;
  if (attr & TERM_BNW) attr = TERM_BNW;
  flags = 0x0;
 }
 if (is_older_than(c->e->version, 4, 7, 3, 0, 0, 0)) {
  switch (attr) {
  case TERM_OLD3_BNW: attr = TERM_BNW; break;
  case TERM_OLD3_BNWM: attr = TERM_BNWM; break;
  case TERM_OLD3_BNWSR: attr = TERM_BNWSR; break;
  case TERM_OLD3_BNWKS: attr = TERM_BNWKS; break;
  case TERM_OLD3_BNWKS2: attr = TERM_BNWKS2; break;
  case TERM_OLD3_PVPBB: attr = TERM_PVPBB; break;
  case TERM_OLD3_PVP: attr = TERM_PVP; break;
  }
 }
 switch (attr) {
 case TERM_BNW: return(colour_randint(c,2) < 2 ? TERM_L_DARK : TERM_WHITE);
 case TERM_BNWM:
  switch (colour_randint(c,3)) {
  case 1: return(TERM_L_DARK);
  case 2: return(TERM_WHITE);
  case 3: return(animated_colour(c,TERM_HOLYFIRE));
  }
  __attribute__ ((fallthrough));
 case TERM_BNWSR:
  switch (colour_randint(c,3)) {
  case 1: return(TERM_L_DARK);
  case 2: return(TERM_WHITE);
  case 3: return(TERM_BLUE);
  }
  __attribute__ ((fallthrough));
 case TERM_BNWKS:
  switch (colour_randint(c,3)) {
  case 1: return(TERM_L_DARK);
  case 2: return(TERM_WHITE);
  case 3: return(animated_colour(c,TERM_PSI));
  }
  __attribute__ ((fallthrough));
 case TERM_BNWKS2:
  switch (colour_randint(c,3)) {
  case 1: return(TERM_L_DARK);
  case 2: return(TERM_WHITE);
  case 3: return(TERM_ORANGE);
  }
  __attribute__ ((fallthrough));
 case TERM_PVPBB:
  switch (colour_randint(c,3)) {
  case 1: return(TERM_L_DARK);
  case 2: return(TERM_SLATE);
  case 3: return(TERM_YELLOW);
  }
  __attribute__ ((fallthrough));
 case TERM_PVP:
  switch (colour_randint(c,3)) {
  case 1: return(TERM_L_DARK);
  case 2: return(TERM_L_RED);
  case 3: return(TERM_YELLOW);
  }
 }
 if (attr == TERM_SHIELDM) {
  switch (colour_randint(c,3)) {
  case 1: return(TERM_VIOLET);
  case 2: return(TERM_L_RED);
  case 3: return(TERM_ORANGE);
  }
 }
 if (attr == TERM_SHIELDI) {
  switch (colour_randint(c,5)) {
  case 1: return(TERM_L_RED);
  case 2: return(TERM_L_GREEN);
  case 3: return(TERM_L_BLUE);
  case 4: return(TERM_YELLOW);
  case 5: return(TERM_VIOLET);
  }
 }
 switch (attr) {
 case TERM_MULTI:
  return(colour_randint(c,15));
 case TERM_FIRE:
  return(colour_randint(c,7) > 6 ? TERM_YELLOW : colour_random(c,3) > 1 ? TERM_RED : TERM_L_RED);
 case TERM_POIS:
  return(colour_randint(c,6) > 4 ? TERM_GREEN : TERM_L_GREEN);
 case TERM_COLD:
  return(colour_randint(c,5) > 3 ? TERM_WHITE : TERM_L_WHITE);
 case TERM_ELEC:
  return(colour_randint(c,7) > 6 ? TERM_WHITE : (colour_randint(c,4) == 1 ? TERM_L_BLUE : TERM_BLUE));
 case TERM_HALF:
  return(shimmer(c));
 case TERM_ACID:
  return(colour_randint(c,5) > 4 ? TERM_L_DARK : TERM_SLATE);
 case TERM_CONF:
  return(colour_randint(c,5) > 3 ? TERM_UMBER : TERM_L_UMBER);
 case TERM_SOUN:
  return(colour_randint(c,5) > 3 ? TERM_L_UMBER : TERM_YELLOW);
 case TERM_SHAR:
  return(colour_randint(c,5) > 3 ? TERM_UMBER : TERM_SLATE);
 case TERM_LITE:
  return(colour_randint(c,5) > 3 ? TERM_ORANGE : TERM_YELLOW);
 case TERM_DARKNESS:
  return(colour_randint(c,5) > 4 ? TERM_SLATE : TERM_L_DARK);
 case TERM_CURSE:
  return(colour_randint(c,2) == 1 ? (colour_randint(c,5) > 4 ? TERM_SLATE : TERM_L_DARK) : TERM_L_DARK);
 case TERM_ANNI:
  return(colour_randint(c,2) == 1 ? (colour_randint(c,5) > 4 ? TERM_SLATE : TERM_L_DARK) : TERM_L_DARK);
 case TERM_PSI:
  return(colour_randint(c,5) != 1 ? (colour_random(c,2) ? (colour_random(c,2) ? TERM_YELLOW : TERM_L_BLUE) : TERM_L_WHITE) : TERM_WHITE);
 case TERM_NEXU:
  return(colour_randint(c,5) < 3 ? TERM_L_RED : TERM_VIOLET);
 case TERM_NETH:
  return(colour_randint(c,4) == 1 ? TERM_L_GREEN : TERM_L_DARK);
 case TERM_DISE:
  return(colour_randint(c,4) != 1 ? TERM_ORANGE : TERM_BLUE);
 case TERM_INER:
  return(colour_randint(c,5) < 3 ? TERM_SLATE : TERM_L_WHITE);
 case TERM_FORC:
  return(colour_randint(c,5) < 3 ? TERM_L_WHITE : TERM_ORANGE);
 case TERM_GRAV:
  return(colour_randint(c,3) == 1 ? TERM_L_UMBER : TERM_SLATE);
 case TERM_TIME:
  return(colour_randint(c,3) == 1 ? TERM_L_GREEN : TERM_L_BLUE);
 case TERM_METEOR:
  return(colour_randint(c,3) == 1 ? TERM_RED : TERM_UMBER);
 case TERM_MANA:
  return(colour_randint(c,5) != 1 ? TERM_VIOLET : TERM_L_BLUE);
 case TERM_WATE:
  return(colour_randint(c,20) == 1 ? TERM_L_BLUE : TERM_BLUE);
 case TERM_ICE:
  return(colour_randint(c,4) == 1 ? TERM_L_BLUE : TERM_WHITE);
 case TERM_PLAS:
  return(colour_randint(c,5) == 1 ? TERM_RED : TERM_L_RED);
 case TERM_DETO:
  return(colour_randint(c,6) < 4 ? TERM_L_RED : (colour_randint(c,4) == 1 ? TERM_RED : TERM_L_UMBER));
 case TERM_DISI:
  return(colour_randint(c,3) != 1 ? TERM_L_DARK : (colour_randint(c,2) == 1 ? TERM_ORANGE : TERM_VIOLET));
 case TERM_NUKE:
  return((colour_randint(c,2)==1?TERM_RED:TERM_GREEN));
 case TERM_UNBREATH:
  return(colour_randint(c,7) < 3 ? TERM_L_GREEN : TERM_GREEN);
 case TERM_HOLYORB:
  return(colour_randint(c,6) == 1 ? TERM_ORANGE : TERM_L_DARK);
 case TERM_HOLYFIRE:
  return(colour_randint(c,3) != 1 ? TERM_ORANGE : (colour_randint(c,2) == 1 ? TERM_YELLOW : TERM_WHITE));
 case TERM_HELLFIRE:
  return(colour_randint(c,5) == 1 ? TERM_RED : TERM_L_DARK);
 case TERM_THUNDER:
  return(colour_randint(c,3) != 1 ? (colour_randint(c,7) > 6 ? TERM_WHITE : (colour_randint(c,4) == 1 ? TERM_L_BLUE : TERM_BLUE))
      : (colour_randint(c,2) == 1 ? TERM_YELLOW : (colour_randint(c,5) > 3 ? TERM_ORANGE : TERM_YELLOW)));
 case TERM_LAMP:
  if (c->e->lamp_fainting) return(c->e->lamp_bright ? TERM_L_UMBER : TERM_UMBER);
  return(c->e->lamp_bright ? TERM_YELLOW : TERM_ORANGE);
 case TERM_LAMP_DARK:
  if (c->e->lamp_fainting) return(c->e->lamp_bright ? TERM_UMBER : TERM_L_DARK);
  return(c->e->lamp_bright ? TERM_L_UMBER : TERM_UMBER);
 case TERM_EMBER:
  return(colour_random(c,6) ? TERM_RED : (colour_random(c,2) ? TERM_ORANGE : TERM_L_RED));
 case TERM_STARLITE:
  return(colour_random(c,16) ? TERM_WHITE : (colour_random(c,4) ? TERM_BLUE : TERM_L_BLUE));
 case TERM_HAVOC:
  switch (colour_random(c,9)) {
  case 0: case 1:
   return(TERM_VIOLET);
  case 3: return(TERM_ORANGE);
  case 4: return(TERM_L_DARK);
  default:
   return(animated_colour(c,TERM_DETO));
  }
 case TERM_FIRETHIN:
  return(!colour_random(c,2) ? (colour_random(c,2) ? TERM_YELLOW : (colour_random(c,2) ? TERM_L_RED : TERM_RED)) : TERM_DARK);
 case TERM_SELECTOR:
  switch ((unsigned)((int64_t)(c->e->now_ms/100)) % 14) {
  case 0: case 1:
  case 14: case 15:
   return(TERM_UMBER);
  case 2: case 3:
  case 12: case 13:
   return(TERM_ORANGE);
  case 4: case 5: case 6:
  case 7: case 8:
  case 9: case 10: case 11:
   return(TERM_YELLOW);
  }
  __attribute__ ((fallthrough));
 case TERM_SMOOTHPAL:
  switch ((((unsigned)((int64_t)(c->e->now_ms/100)) * 10) / 52) % 6) {
  case 0: return(TERM_L_RED);
  case 1: return(TERM_ORANGE);
  case 2: return(TERM_YELLOW);
  case 3: return(TERM_L_GREEN);
  case 4: return(TERM_L_BLUE);
  case 5: return(TERM_VIOLET);
  }
  __attribute__ ((fallthrough));
 case TERM_SEL_RED:
  switch ((unsigned)((int64_t)(c->e->now_ms/100)) % 5) {
  case 0:
   return(TERM_L_RED);
  case 1: case 2:
   return(TERM_RED);
  case 3: case 4:
   return(TERM_L_DARK);
  }
  __attribute__ ((fallthrough));
 case TERM_SEL_BLUE:
  if (true
      ) {
   switch ((unsigned)((int64_t)(c->e->now_ms/100)) % 14) {
   case 0: case 1:
   case 14: case 15:
    return(TERM_BLUE);
   case 2: case 3:
   case 12: case 13:
    return(TERM_SLATE);
   case 7: case 8:
   case 4: case 5: case 6:
   case 9: case 10: case 11:
    return(TERM_L_BLUE);
   }
  } else {
   return(TERM_L_BLUE);
  }
  __attribute__ ((fallthrough));
 case TERM_SRCLITE: return(searchlight(c));
 case TERM_SEL_GREEN:
  switch ((unsigned)((int64_t)(c->e->now_ms/100)) % 14) {
  case 0: case 1:
  case 14: case 15:
   return(TERM_L_DARK);
  case 2: case 3:
  case 12: case 13:
   return(TERM_GREEN);
  case 4: case 5: case 6:
  case 7: case 8:
  case 9: case 10: case 11:
   return(TERM_L_GREEN);
  }
  __attribute__ ((fallthrough));
 case TERM_ANIM_WATER_EAST:
  if (!(c->x+13)) return(animated_colour(c,TERM_WATE));
  return(((c->x+13) - ((int64_t)(c->e->now_ms/100)) / 1) % 5 ? animated_colour(c,TERM_WATE) : (colour_random(c,4) ? TERM_L_BLUE : TERM_BLUE));
 case TERM_ANIM_WATER_WEST:
  if (!(c->x+13)) return(animated_colour(c,TERM_WATE));
  return(((c->x+13) + ((int64_t)(c->e->now_ms/100)) / 1) % 5 ? animated_colour(c,TERM_WATE) : (colour_random(c,4) ? TERM_L_BLUE : TERM_BLUE));
 case TERM_ANIM_WATER_NORTH:
  if (!(c->x+13)) return(animated_colour(c,TERM_WATE));
  return(((c->y+1) + ((int64_t)(c->e->now_ms/100)) / 1) % 5 ? animated_colour(c,TERM_WATE) : (colour_random(c,4) ? TERM_L_BLUE : TERM_BLUE));
 case TERM_ANIM_WATER_SOUTH:
  if (!(c->x+13)) return(animated_colour(c,TERM_WATE));
  return(((c->y+1) - ((int64_t)(c->e->now_ms/100)) / 1) % 5 ? animated_colour(c,TERM_WATE) : (colour_random(c,4) ? TERM_L_BLUE : TERM_BLUE));
 case TERM_ANIM_LAVA_EAST:
  if (!(c->x+13)) return(animated_colour(c,TERM_FIRE));
  return(((c->x+13) - ((int64_t)(c->e->now_ms/100)) / 1) % 5 ? animated_colour(c,TERM_FIRE) : (colour_random(c,4) ? TERM_YELLOW : TERM_ORANGE));
 case TERM_ANIM_LAVA_WEST:
  if (!(c->x+13)) return(animated_colour(c,TERM_FIRE));
  return(((c->x+13) + ((int64_t)(c->e->now_ms/100)) / 1) % 5 ? animated_colour(c,TERM_FIRE) : (colour_random(c,4) ? TERM_YELLOW : TERM_ORANGE));
 case TERM_ANIM_LAVA_NORTH:
  if (!(c->x+13)) return(animated_colour(c,TERM_FIRE));
  return(((c->y+1) + ((int64_t)(c->e->now_ms/100)) / 1) % 5 ? animated_colour(c,TERM_FIRE) : (colour_random(c,4) ? TERM_YELLOW : TERM_ORANGE));
 case TERM_ANIM_LAVA_SOUTH:
  if (!(c->x+13)) return(animated_colour(c,TERM_FIRE));
  return(((c->y+1) - ((int64_t)(c->e->now_ms/100)) / 1) % 5 ? animated_colour(c,TERM_FIRE) : (colour_random(c,4) ? TERM_YELLOW : TERM_ORANGE));
 default:
  return(TERM_L_WHITE);
 }
}
