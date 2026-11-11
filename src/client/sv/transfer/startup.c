#include "transfer/startup.h"
#define CLIENT_SIDE
#define USE_SDL3
#ifndef MINGW
#define LINUX
#endif
#include "../../../common/defines.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    SvTransfer *files;
    SvLuaReload *lua;
    uint64_t generation;
    bool begun;
    SvLuaResult begin_result;
} SvTransferStartup;
static void diagnostic(void *context, const char *message)
{
    (void)context;
    fprintf(stderr, "SV transfer: %s\n", message);
}

static SvResult receive_flags(void *context, const uint32_t flags[4])
{
    SvTransferStartup *startup = context;
    if (startup->begun) return SV_OK;
    SvLuaFeatures features = {
        .rpg = (flags[3] & SFLG0_RPG) != 0,
        .arcade = (flags[3] & SFLG0_ARCADE) != 0,
        .fun = (flags[3] & SFLG0_FUN) != 0,
        .party = (flags[3] & SFLG0_PARTY) != 0,
        .test = (flags[3] & SFLG0_TEST) != 0,
        .temporary = flags[1]
    };
    startup->begin_result = sv_lua_reload_begin(startup->lua, startup->generation, features);
    startup->begun = true;
    /* A transfer may repair the scripts that failed to open. C011 consumes the
     * independent begin result instead of hiding it or inventing readiness. */
    return SV_OK;
}
static SvResult receive_file(void *context, const void *bytes, size_t size, size_t *consumed)
{
    SvTransferStartup *startup = context;
    if (!startup->begun) return SV_INVALID;
    return sv_transfer_receive(startup->files, startup->generation, bytes, size, consumed);
}
static SvResult pump(void *context)
{
    SvTransferStartup *startup = context;
    return startup->begun ? sv_transfer_pump(startup->files, startup->generation) : SV_OK;
}
static SvOutput take_output(void *context, void *bytes, size_t capacity)
{
    return sv_transfer_take_output(((SvTransferStartup *)context)->files, bytes, capacity);
}
static SvTransferSummary summary(const void *context)
{
    return sv_transfer_summary(((const SvTransferStartup *)context)->files);
}
static SvLuaResult begin_result(const void *context)
{
    return ((const SvTransferStartup *)context)->begin_result;
}
static SvResult upload(void *context, const char *filename)
{
    SvTransferStartup *startup = context;
    if (!startup->begun) return SV_INVALID;
    return sv_transfer_upload(startup->files, startup->generation, filename);
}
static bool create(void *context, uint64_t generation, const int version[6], SvLoginTransfer *out)
{
    const SvTransferStartupConfig *config = context;
    if (!config || !config->library || !config->lua || !out) return false;
    SvTransferStartup *startup = calloc(1, sizeof(*startup));
    if (!startup) return false;
    startup->files = sv_transfer_create(config->library);
    startup->lua = config->lua;
    startup->generation = generation;
    startup->begin_result.status = SV_LUA_NOT_READY;
    if (startup->files) sv_transfer_set_diagnostic(startup->files,
        config->diagnostic ? config->diagnostic : diagnostic, config->diagnostic_context);
    if (!startup->files || sv_transfer_begin(startup->files, generation, version,
                                            config->lua, config->no_updates) != SV_OK) {
        sv_transfer_destroy(startup->files);
        free(startup);
        return false;
    }
    *out = (SvLoginTransfer){startup, receive_flags, receive_file, pump, take_output,
                            summary, begin_result, upload};
    return true;
}
static void destroy(SvLoginTransfer *attachment)
{
    if (!attachment || !attachment->context) return;
    SvTransferStartup *startup = attachment->context;
    sv_transfer_close(startup->files, startup->generation);
    if (startup->begun) sv_lua_reload_close(startup->lua, startup->generation);
    sv_transfer_destroy(startup->files);
    free(startup);
    *attachment = (SvLoginTransfer){0};
}
SvLoginTransferFactory sv_transfer_startup_factory(SvTransferStartupConfig *config)
{
    return (SvLoginTransferFactory){config, create, destroy};
}
