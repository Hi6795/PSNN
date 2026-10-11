# PSNN — Offline Core & Diagnostics v0.1.0

Status: **Architecture approved 2026-10-11; host-side implementation complete. PS4 execution unverified.**

## Goal

Create a standalone PS4 homebrew foundation for **PlayStation No Network (PSNN)** that supports safe local-only identity, session state and diagnostics. Future opt-in, title-specific adapters can integrate supported game features without Sony PSN sign-in, system-wide hooks or network dependency. Intended console test targets: PS4 firmware 11.02 and 13.52.

## Critical semantic boundaries

- A PSNN local identity is not a PSN identity and does not grant access to Sony PSN, PlayStation Store, cloud services, purchased entitlements, subscriptions, account IDs, trophy synchronization, or online-exclusive game backends.
- A PSNN local session is **in-process demonstration state**. It is not equivalent to `sceNp` successful sign-in, a Sony service ticket, a game backend session, or a friend/presence service.
- No memory patches, native PSN stubs, eboot modification, privilege escalation, DRM bypass, or network requests in v0.1.0.
- Default plugin entry does not write files, perform network activity, install hooks, or call into game APIs. It is an **ABI-only, opt-in shim** rather than a working game integration.
- Console hardware validation is required before claiming PS4 compatibility; host builds do not prove console loadability.

## Components

1. `psnn_core`: portable C++11 class for profile persistence, selection, ephemeral local session state, diagnostics. No daemon thread.
2. `psnn` CLI: explicitly initializes a storage root and runs profile/diagnostic commands. Default path `/data/PSNN`; host users can supply `--root /tmp/...`.
3. `adapter_api.h`: stable C ABI v1 for future game adapters, with explicit `local_session`/`network_enabled` bits and a title ID; no dynamic loader in milestone 1.
4. `ps4/plugin_entry.c`: diagnostic-only GoldHEN-style public lifecycle symbols and offline status query; NOT yet linked as a PS4 PRX.
5. Unit/functional tests: host-side persistence, duplicate session rejection, corrupt data handling and diagnostics.

## Data layout

```
/data/PSNN/
  profiles.psnn       # PSNN_PROFILES_V1; LOCAL-########|Name
  active.psnn         # PSNN_ACTIVE_V1; selected local ID, optional
```

All created files are intended to be owner-only (mode 0600). Directories are created explicitly at `init` using mode 0700. No `/data/PSNN/logs` or `/data/PSNN/update` directories are created, and there is no startup file-backed logger.

The profile file maxes at 32 KiB, limits 64 profiles, restricts names to a basic ASCII character set and rejects invalid records. Writes use same-directory temporary file, sync, and rename to avoid accidental truncation on write failure. A corrupt existing file prevents startup rather than being overwritten.

## Workflows

1. `init`: create and validate safe storage root; if profile file absent, create an empty one; otherwise load without overwriting.
2. `profile add`: validate, append a local profile record and atomically persist.
3. `profile select`: require existing ID; refuse switching while a local session is active; atomically persist selection.
4. `session demo`: require selected local profile; create and close a nonpersistent in-memory session; output explicit Sony-auth=0/network=0 status.
5. `diag`: collect truthful PASS/INFO/WARN/FAIL checks and perform a temporary file write/read/verify/fsync/delete probe only on direct request. JSON output schema `psnn.diagnostics.v1`.

## Failure policy

- Invalid root and profile names fail closed.
- Unexpected data file format or duplicate profile IDs fail closed.
- Permission failures return an error. No stealth retry, no background folder creation, no deleting user data to repair problems.
- No automatic network fallback, firmware-specific patching, or crash-prone startup probes.
- Loaded runtime state never persists across process restarts.

## Acceptance criteria

- Host builds with CMake and C++11/C11.
- Unit tests pass including restart persistence, ID uniqueness, session transition rejection, corrupt-data protection and storage write probe.
- CLI smoke test produces parseable diagnostics JSON with `network=disabled` and Sony authentication explicitly not implemented.
- Diagnostic shim exports correct lifecycle/query symbols in a host `.so`; this is a syntax/ABI sanity check only.
- PS4 11.02/13.52 tests remain **blocked** until a PS4 toolchain, actual PRX integration, and console evidence are available.

## Future milestones (not included)

- PS4 native builder and PRX package with verified runtime link/dependencies.
- Game-scoped adapter loader and per-title allowlist, diagnostics-only mode first.
- Local profile interface (homebrew UI), opt-in local services and LAN transport.
- Explicit per-game compatibility implementation, with hardware tests on each firmware and target title.
