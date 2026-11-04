#ifndef SV_MOTD_COLORS_H
#define SV_MOTD_COLORS_H
#include <stdint.h>
/* Isolated baseline flick_colour/anim2static for MOTD's formatting roles.
 * Gameplay flags, weather, lamp fuel and map coordinates do not exist here.
 * Ticks are 100 ms, as FL_SPEED; randomness is local to each cell and tick. */
static inline unsigned sv_motd_random(uint32_t *state,unsigned limit)
{
    *state=*state*1664525u+1013904223u;
    return (*state>>8)%limit;
}
static inline unsigned sv_motd_color(unsigned role,uint32_t ticks,uint32_t seed,int reduced)
{
    static const unsigned char still[]={
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        13,4,2,14,9,12,10,11,15,11,7,8,12,10,
        11,10,8,3,2,9,2,13,7,10,8,6,1,12,12,5,5,3,3,4,14,4,
        1,12,11,15,11,12,12,14,4,13};
    const char *codes="dwsorgbuDWvyRGBUpfaechmLCSHAMIPxnTqFVtENQYilOkKjJXzZ0123456789";
    if(role<16)return role;
    if(role>=sizeof(still))return 9;
    char code=codes[role];
    if(reduced && code!='4' && code!='5' && code!='6' && code!='7' && code!='9')return still[role];
    uint32_t state=seed^(ticks*374761393u);
#define R(n) sv_motd_random(&state,(n))
#define D(n) (R(n)+1)
    switch(code) {
    case 'p':return D(6)>4?5:13;
    case 'f':case '8':return D(7)>6?11:R(3)>1?4:12;
    case 'a':return D(5)>4?8:2;
    case 'e':return D(7)>6?1:D(4)==1?14:6;
    case 'c':return D(5)>3?1:9;
    case 'h': {static const unsigned char shimmer[]={4,12,1,13,6,8,5};return shimmer[R(7)];}
    case 'm':return D(15);
    case 'L':return D(5)>3?3:11;
    case 'C':return D(5)>3?7:15;
    case 'S':return D(5)>3?15:11;
    case 'H':return D(5)>3?7:2;
    case 'A':return D(5)>4?2:8;
    case 'M': {static const unsigned char shield[]={10,12,3};return shield[R(3)];}
    case 'I': {static const unsigned char shield[]={12,13,14,11,10};return shield[R(5)];}
    case 'P':return D(5)!=1?(R(2)?(R(2)?11:14):9):1;
    case 'x':return D(5)<3?12:10;
    case 'n':return D(4)==1?13:8;
    case 'T':return D(4)!=1?3:6;
    case 'q':return D(5)<3?2:9;
    case 'F':return D(5)<3?9:3;
    case 'V':return D(3)==1?15:2;
    case 't':return D(3)==1?13:14;
    case 'E':return D(3)==1?4:7;
    case 'N':return D(5)!=1?10:14;
    case 'Q':return D(3)!=1?8:D(2)==1?3:10;
    case 'Y':return D(20)==1?14:6;
    case 'i':return D(4)==1?14:1;
    case 'l':return D(5)==1?4:12;
    case 'O':return D(6)<4?12:D(4)==1?4:15;
    case 'k':return D(2)==1?4:5;
    case 'K':return D(7)<3?13:5;
    case 'j':return D(6)==1?3:8;
    case 'J':return D(3)!=1?3:D(2)==1?11:1;
    case 'X':return D(5)==1?4:8;
    case 'z':return D(3)!=1?(D(7)>6?1:D(4)==1?14:6):(D(2)==1?11:D(5)>3?3:11);
    case 'Z':return R(6)?4:R(2)?3:12;
    case '0':return R(16)?1:R(4)?6:14;
    case '1':switch(R(9)) {case 0:case 1:return 10;case 3:return 3;case 4:return 8;default:return D(6)<4?12:D(4)==1?4:15;}
    case '2':return R(20)?11:3;
    case '3':return R(20)?15:7;
    case '4':return ticks%14<2?7:ticks%14<4 || ticks%14>=12?3:11;
    case '5': {static const unsigned char smooth[]={12,3,11,13,14,10};return smooth[((ticks*10)/52)%6];}
    case '6':return ticks%5==0?12:ticks%5<3?4:8;
    case '7':return ticks%14<2?6:ticks%14<4 || ticks%14>=12?2:14;
    case '9':return ticks%14<2?8:ticks%14<4 || ticks%14>=12?5:13;
    default:return 9;
    }
#undef R
#undef D
}
#endif
