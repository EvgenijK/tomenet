#ifndef SV_STATUS_UPDATE_H
#define SV_STATUS_UPDATE_H
#include "result.h"
#include "session/status-state.h"
bool sv_status_packet_supported(unsigned char type);
SvResult sv_status_decode(const int version[6], const void *bytes, size_t size,
                          SvHudUpdate *out, size_t *consumed);
#endif
