#ifndef SV_FILE_DNA_H
#define SV_FILE_DNA_H
#include "files/files.h"
#include <stdint.h>
typedef struct { int16_t mode, class_id, race, trait, stats[6]; } SvDna;
bool sv_dna_filename(char out[256], const char *name);
SvFileResult sv_dna_load(SvDna *, const char *user_root, const char *library,
                         const char *name, const char *previous_name);
SvFileResult sv_dna_save(const SvDna *, const char *user_root, const char *name);
#endif
