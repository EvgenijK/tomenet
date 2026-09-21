#ifndef SV_PROTOCOL_H
#define SV_PROTOCOL_H
#include "result.h"
#include "session.h"
typedef struct SvProtocol SvProtocol;
/* Fixed slice limits, observable by callers for backpressure. */
#define SV_PROTOCOL_CAPACITY 1024
SvProtocol *sv_protocol_create(const int version[6]);
void sv_protocol_destroy(SvProtocol *protocol);
/* All-or-nothing append. BACKPRESSURE consumes nothing; caller retains the bytes. */
SvResult sv_protocol_receive(SvProtocol *protocol, const void *bytes, size_t size);
/* One complete input, or WAITING without any model-visible change. */
SvResult sv_protocol_next(SvProtocol *protocol, SvChange *change);
size_t sv_protocol_pending(const SvProtocol *protocol);
size_t sv_protocol_capacity(const SvProtocol *protocol);
SvOutput sv_protocol_output(SvProtocol *protocol, void *bytes, size_t capacity);
#endif
