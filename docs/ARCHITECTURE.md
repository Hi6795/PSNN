# PSNN architecture — core and extension boundaries

## Why the design is game-scoped

PSNN is an independent, opt-in platform for **local** identities and services.
It is not a stand-in for genuine PSN authentication. A single system-wide
hook would create large compatibility and stability risks; the design instead
uses a portable offline core and separate, versioned per-title adapters.

```text
       [Host CLI / future PS4 UI]       [Optional title-specific adapter]
                     |                             |
              psnn::OfflineCore        psnn_adapter_info_v1 / probe
              profiles + diagnostics              |
                     |                 psnn_offline_status_v1
           explicit local files                   |
          (/data/PSNN proposed)        diagnostic-only plugin boundary
```

## Existing interfaces

| Module | Source | Status |
|---|---|---|
| OfflineCore | `include/psnn/core.hpp` / `src/core.cpp` | Built and tested on Linux host |
| CLI | `src/main.cpp` | Built and smoke-tested on Linux host |
| Game adapter C ABI v1 | `include/psnn/adapter_api.h` | Defined; not plugged into games |
| Plugin lifecycle shim | `ps4/plugin_entry.c` | Linux ABI smoke-tested only |
| PS4 storage and linking | `ps4/README.md` | Documented, not verified |

The core's local sessions are intentionally in-memory and non-persistent.
`local_session=1` means **only** a PSNN-local session; it never implies Sony NP
sign-in. ABI v1 reports `network_enabled=0`. Future networking, if approved,
will require a new explicitly documented interface and opt-in policy.

## Storage

`profiles.psnn` and optional `active.psnn` use versioned text formats; the
current implementation allows up to 64 ASCII-name local profiles. `init`
must be explicitly requested; the plugin shim creates no files. The core
currently uses POSIX filesystem APIs and requires platform-specific review
before PS4 linking. Concurrent writers are not supported.

## Dependency direction

Core and tests must not link to GoldHEN, Sony APIs or console-only headers.
Platform entrypoints depend on stable public core/ABI interfaces, not the
reverse. Every adapter must be title-ID scoped, optional, and separately
reviewed on each supported firmware.

## Release gates

1. Linux host tests + CLI smoke pass.
2. Static/export inspection of a genuine PS4 build, when available.
3. Crash-free no-hook plugin load/unload on the target title and firmware.
4. Explicit storage tests only after no-write startup has been validated.
5. Opt-in game adapters only after tests for versions/firmwares they advertise.

A passed Linux CI check does not satisfy gates 2–5.
