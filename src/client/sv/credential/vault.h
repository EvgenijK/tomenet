#ifndef SV_CREDENTIAL_VAULT_H
#define SV_CREDENTIAL_VAULT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SV_VAULT_SECRET_CAPACITY 79
#define SV_VAULT_KEY_CAPACITY 512
typedef enum { SV_VAULT_PENDING, SV_VAULT_FOUND, SV_VAULT_MISSING,
               SV_VAULT_UNAVAILABLE, SV_VAULT_INVALID, SV_VAULT_SAVED } SvVaultResult;
typedef struct SvVaultRequest SvVaultRequest;

/* Identity uses original address/account bytes. The printable key is a
 * lowercase-hex encoding of three length-prefixed fields under this namespace. */
bool sv_vault_key(char out[SV_VAULT_KEY_CAPACITY], const void *server,
                  size_t server_size, uint16_t port, const void *account,
                  size_t account_size);
SvVaultRequest *sv_vault_lookup(const char *key, uint64_t generation);
SvVaultRequest *sv_vault_store(const char *key, uint64_t generation,
                               const void *secret, size_t secret_size);
/* Returns PENDING until the worker finishes. FOUND copies exact bytes, with
 * no encoding projection. A stale generation is discarded and returns INVALID. */
SvVaultResult sv_vault_poll(SvVaultRequest **request, uint64_t generation,
                            void *secret, size_t capacity, size_t *size);
/* Cancellation does not wait for an OS unlock dialog; the worker later wipes
 * its own buffers and frees the detached request. */
void sv_vault_cancel(SvVaultRequest **request);
#endif
