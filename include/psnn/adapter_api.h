#ifndef PSNN_ADAPTER_API_H
#define PSNN_ADAPTER_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Informational, versioned contract for future opt-in game adapters.
 * No automatic hooking, imports, socket calls, PSN token creation or title patches.
 */
#define PSNN_ADAPTER_ABI_V1 1u

typedef struct psnn_adapter_info_v1 {
    uint32_t struct_size;
    uint32_t abi_version;
    const char *title_id;       /* e.g., CUSAxxxxx; exact supported title ID */
    const char *adapter_name;
    uint32_t flags;             /* reserved: must be 0 in v1 */
} psnn_adapter_info_v1;

typedef struct psnn_offline_status_v1 {
    uint32_t struct_size;
    uint32_t abi_version;
    uint32_t local_session;     /* 1 = local only, never Sony authentication */
    uint32_t network_enabled;   /* v1 MUST be 0 */
} psnn_offline_status_v1;

typedef int32_t (*psnn_adapter_probe_v1)(
    const psnn_offline_status_v1 *status,
    psnn_adapter_info_v1 *info);

#ifdef __cplusplus
}
#endif
#endif
