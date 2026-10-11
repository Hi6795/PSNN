# PSNN 0.1.0 — Verification report (2026-10-11)

## Environment

- Runner: Linux x86_64 container
- Build system: CMake
- Host compiler: GNU GCC/G++ 14.2.0
- PS4 toolchain: **not installed in this environment**
- PS4 console: **not accessible for tests in this run**

## Commands actually run

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
./scripts/smoke.sh
nm -D build/libpsnn_plugin_shim.so
```

## Observed results

- Host compilation: success; built `psnn`, `libpsnn_core.a`, `psnn_tests`, `libpsnn_plugin_shim.so`.
- CTest: **2/2 tests passed**, 0 failures (`offline_core`, `plugin_shim_host_abi`).
- Core test performs 25+ assertions covering invalid root, profile creation, persistence, selection, session transitions, corruption refusal, diagnostics.
- CLI smoke: `PSNN_READY`, `PSNN_PROFILE_CREATED`, `PSNN_PROFILE_SELECTED`, `PSNN_CURRENT`, `PSNN_LOCAL_SESSION_OPEN ... sony_auth=0 network=0`, `PSNN_LOCAL_SESSION_CLOSED`.
- Smoke JSON parse and checks: passed, including temporary file write/read/verify/fsync/delete probe; diagnostics confirmed Sony authentication unavailable and network disabled.
- Host shim exported `g_pluginName`, `plugin_load`, `plugin_unload`, `psnn_query_offline_status_v1`; ABI test confirmed lifecycle transitions and offline status (`local_session=0`, `network_enabled=0`).

## Not verified — NOT READY for console deployment

- GoldHEN/OpenOrbis PS4 PRX build and link
- Actual PRX startup/stop in game on firmware 11.02 or 13.52
- Filesystem write permissions within a game plugin sandbox
- Game-specific API imports, title version/firmware compatibility
- Real NP/PSN sign-in, friends, multiplayer, Sony service compatibility
- Remote networking, telemetry, update service

This report only establishes **host-side correctness of the current standalone core and ABI shim**.
