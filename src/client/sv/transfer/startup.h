#ifndef SV_TRANSFER_STARTUP_H
#define SV_TRANSFER_STARTUP_H
#include "protocol/login.h"
/* Application config is borrowed through the factory lifetime. The adapter
 * owns its transfer registry; Lua belongs to the application across attempts. */
typedef struct {
    const char *library;
    SvLuaReload *lua;
    bool no_updates;
    SvTransferDiagnostic diagnostic;
    void *diagnostic_context;
} SvTransferStartupConfig;
SvLoginTransferFactory sv_transfer_startup_factory(SvTransferStartupConfig *);
#endif
