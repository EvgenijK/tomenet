#ifndef SV_MACRO_EXECUTOR_H
#define SV_MACRO_EXECUTOR_H
#include "input/command.h"
#include "input/macros.h"

typedef struct { uint64_t confirmation_owner; } SvMacroExecution;
typedef struct {
    SvMacroSet *macros;
    SvMacroRunner *runner;
    SvInputRouter *input;
    const SvInputBindings *bindings;
    SvCommandRouter *command;
    SvProtocol *protocol;
    SvMacroExecution *state;
} SvMacroExecutor;
typedef struct {
    SvResult result;
    bool pump;
    unsigned char key;
} SvMacroPhysicalRoute;

void sv_macro_executor_reset(SvMacroExecutor *executor);
bool sv_macro_executor_idle(const SvMacroExecutor *executor);
bool sv_macro_executor_active(const SvMacroExecutor *executor);
bool sv_macro_executor_extended_waiting(const SvMacroExecutor *executor);
bool sv_macro_executor_waiting(const SvMacroExecutor *executor);
void sv_macro_executor_key_request(SvMacroExecutor *executor);
SvResult sv_macro_executor_frame(SvMacroExecutor *executor, SvKeyRequest request,
    SvInputContext context, bool paused, uint64_t now_ms, size_t budget);
SvMacroPhysicalRoute sv_macro_executor_physical(SvMacroExecutor *executor,
    const unsigned char *bytes, size_t size, bool prompt, uint64_t now_ms);
SvResult sv_macro_executor_accept(SvMacroExecutor *executor, unsigned char key,
                                  uint64_t now_ms);
#endif
