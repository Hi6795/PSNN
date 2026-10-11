# PS4 integration boundary — not yet console executable

`plugin_entry.c` is a minimal, opt-in **GoldHEN-style source shim**. It exports
`g_pluginName`, `g_pluginDesc`, `g_pluginAuth`, `g_pluginVersion`, `plugin_load`,
`plugin_unload`, `module_start`, `module_stop` and a read-only
`psnn_query_offline_status_v1` function. `plugin_load` does **not** initialize
profiles, create directories, hook functions, invoke Sony NP APIs, or simulate
Sony's sign-in state.

The host CMake target (`libpsnn_plugin_shim.so`) verifies C syntax and exported
symbols. A Linux `.so` **cannot be placed on the PS4** as a `.prx`.

To make a real opt-in PS4 build, a maintainer must:

1. Install the upstream OpenOrbis SDK and GoldHEN Plugin SDK (licensed separately).
2. Integrate `plugin_entry.c` with the upstream GoldHEN plugin template build
   system, using the correct PS4 CRT, linker scripts, export visibility and
   target-specific OpenOrbis build options.
3. Generate an actual PS4 PRX and inspect dependencies/exports using PS4 tools.
4. Run hardware-only **no-hook, no-file-write** boot/unload testing on each
   requested firmware before connecting the portable offline core.
5. Integrate storage only in an execution context known to allow `/data/PSNN`
   reads/writes; refuse gracefully when denied.
6. Only after verified safe unload, introduce per-game adapter allowlists.

Useful upstream references:
- https://github.com/OpenOrbis/OpenOrbis-PS4-Toolchain
- https://github.com/GoldHEN/GoldHEN_Plugins_SDK
- https://github.com/GoldHEN/GoldHEN_Plugins_Repository/tree/main/plugin_src/plugin_template

Do not load this source in a game, treat it as a compiled PRX, or assume that
firmware compatibility has been verified.
