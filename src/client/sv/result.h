#ifndef SV_RESULT_H
#define SV_RESULT_H
#include <stddef.h>
typedef enum {
    SV_OK, SV_WAITING, SV_RECOVERED, SV_BACKPRESSURE, SV_CLOSED, SV_STALE,
    SV_BUSY, SV_INVALID, SV_INPUT_OVERFLOW, SV_OUTPUT_OVERFLOW, SV_DECODE_ERROR,
    SV_NO_MEMORY, SV_OUTPUT_TOO_SMALL, SV_EVENT_OVERFLOW
} SvResult;
/* For output, size is bytes copied, or required capacity on OUTPUT_TOO_SMALL. */
typedef struct { SvResult result; size_t size; } SvOutput;
const char *sv_result_text(SvResult result);
#endif
