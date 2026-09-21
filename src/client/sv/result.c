#include "result.h"
const char *sv_result_text(SvResult result)
{
    switch (result) {
    case SV_OK: return "Ready";
    case SV_WAITING: return "Waiting for input";
    case SV_RECOVERED: return "Unknown packet: redraw requested";
    case SV_BACKPRESSURE: return "Input paused: process buffered updates";
    case SV_CLOSED: return "Session closed";
    case SV_STALE: return "Result belongs to an old session";
    case SV_BUSY: return "Session is processing an input";
    case SV_INVALID: return "Invalid input";
    case SV_INPUT_OVERFLOW: return "Session closed: input exceeds buffer limit";
    case SV_OUTPUT_OVERFLOW: return "Session closed: output buffer exhausted";
    case SV_DECODE_ERROR: return "Session closed: packet decode failed";
    case SV_NO_MEMORY: return "Session could not allocate storage";
    case SV_OUTPUT_TOO_SMALL: return "Output destination too small";
    }
    return "Unknown error";
}
