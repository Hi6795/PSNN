/* PSNN v0.1.0 GoldHEN-style plugin entry SHIM.
 * Demonstrates public lifecycle symbols ONLY. Does not attach to game code,
 * load PSNN C++ core, hook Sony APIs, touch system memory or access storage.
 * Host compilation checks ABI symbols, NOT PS4 compatibility.
 * PS4 linker/CRT wiring must be validated against a real OpenOrbis toolchain.
 */
#include <stdint.h>
#include <stddef.h>
#include "psnn/adapter_api.h"

#if defined(__GNUC__)
#define PSNN_PUBLIC __attribute__((visibility("default")))
#define PSNN_HIDDEN __attribute__((visibility("hidden")))
#else
#define PSNN_PUBLIC
#define PSNN_HIDDEN
#endif

PSNN_PUBLIC const char *g_pluginName = "PSNN";
PSNN_PUBLIC const char *g_pluginDesc = "PSNN offline diagnostic shim (no authentication)";
PSNN_PUBLIC const char *g_pluginAuth = "PSNN Project";
PSNN_PUBLIC uint32_t g_pluginVersion = 0x00000100;
static uint32_t g_loaded = 0;

PSNN_PUBLIC int32_t plugin_load(int32_t argc, const char *argv[]) {
    (void)argc; (void)argv;
    g_loaded = 1;
    return 0;
}

PSNN_PUBLIC int32_t plugin_unload(int32_t argc, const char *argv[]) {
    (void)argc; (void)argv;
    g_loaded = 0;
    return 0;
}

/* Returns truthful diagnostic status. local_session=0 because the shim
 * is not connected to a game or the full offline core yet. */
PSNN_PUBLIC int32_t psnn_query_offline_status_v1(psnn_offline_status_v1 *out) {
    if (out == NULL || out->struct_size < sizeof(psnn_offline_status_v1)) return -1;
    out->struct_size = sizeof(psnn_offline_status_v1);
    out->abi_version = PSNN_ADAPTER_ABI_V1;
    out->local_session = 0;
    out->network_enabled = 0;
    return g_loaded ? 0 : 1; /* 1 = module not loaded */
}

PSNN_HIDDEN int32_t module_start(int64_t argc, const void *args) {
    (void)argc; (void)args;
    return 0;
}
PSNN_HIDDEN int32_t module_stop(int64_t argc, const void *args) {
    (void)argc; (void)args;
    return 0;
}
