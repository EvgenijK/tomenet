#ifndef SV_CHARACTER_SETUP_H
#define SV_CHARACTER_SETUP_H
#include <stdint.h>
#define SV_CONTACT_NAME_CAPACITY 80
#define SV_CONTACT_MOTD_CAPACITY (120 * 23)
typedef struct {
    char title[SV_CONTACT_NAME_CAPACITY];
    int16_t adjustment[6];
    uint32_t choice;
} SvContactRace;
typedef struct {
    char title[SV_CONTACT_NAME_CAPACITY];
    int16_t adjustment[6];
    uint8_t recommendation[6];
    uint8_t hidden;
    int32_t base_class;
} SvContactClass;
typedef struct { char title[SV_CONTACT_NAME_CAPACITY]; uint32_t choice; } SvContactTrait;
typedef struct {
    uint16_t frames_per_second;
    uint8_t race_count, class_count, trait_count;
    uint32_t setup_size, motd_size, creation_flags;
    SvContactRace races[255];
    SvContactClass classes[255];
    SvContactTrait traits[255];
    unsigned char motd[SV_CONTACT_MOTD_CAPACITY + 1];
} SvContactSetup;
#endif
