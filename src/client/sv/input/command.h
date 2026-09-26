#ifndef SV_COMMAND_H
#define SV_COMMAND_H
#include "result.h"
#include <stdbool.h>
#include <stdint.h>

/* One-byte packet, text and NUL must fit the SV output queue atomically. */
#define SV_COMMAND_TEXT 1023
typedef enum {
    SV_COMMAND_NONE, SV_COMMAND_RAW, SV_COMMAND_WALK, SV_COMMAND_RUN,
    SV_COMMAND_TUNNEL, SV_COMMAND_STAND, SV_COMMAND_CHAT
} SvCommandKind;
typedef struct {
    SvCommandKind kind;
    unsigned char key, direction;
    unsigned char text[SV_COMMAND_TEXT];
    size_t size;
} SvCommand;
typedef struct {
    bool roguelike, bypass, control, chat;
    bool override[128];
    unsigned char override_command[128], override_direction[128];
    SvCommandKind direction_pending;
    unsigned char text[SV_COMMAND_TEXT];
    size_t text_size;
} SvCommandRouter;

/* The caller copies the router and commits it after output serialization. */
SvResult sv_command_key(SvCommandRouter *router, unsigned char key, SvCommand *output);
SvResult sv_command_override(SvCommandRouter *router, unsigned char key,
                             unsigned char command, unsigned char direction);
#endif
