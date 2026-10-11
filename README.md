# PSNN — PlayStation No Network

**Version:** 0.1.0 — Host-verified offline core and diagnostics.

**Community source repository:** https://github.com/Hi6795/PSNN.
Contributions are welcome via issues and pull requests. See
[CONTRIBUTING](CONTRIBUTING.md), [ARCHITECTURE](docs/ARCHITECTURE.md)
and [ROADMAP](docs/ROADMAP.md).

PSNN is an independent homebrew foundation for **local-only profiles and
sessions**. It is designed toward eventually supporting opted-in adapters for
specific games on jailbroken PS4 systems. The initial targets are firmware
11.02 and 13.52, but there has been **NO PS4 hardware testing** and no PRX
was built.

**PSNN does not sign into PSN, spoof Sony identities, access Sony services,
unlock content, or provide a working game server.** The local session
exists inside the current CLI process only. The diagnostic plugin shim
contains exported lifecycle symbols but currently does no game integration.

## Host build and test

```sh
bash scripts/verify.sh
```

You can also run `./scripts/smoke.sh` after building.

## Local-only CLI demonstration (host Linux)

```sh
./build/psnn --root /tmp/psnn-example init
./build/psnn --root /tmp/psnn-example profile add "Example Player"
./build/psnn --root /tmp/psnn-example profile select LOCAL-00000001
./build/psnn --root /tmp/psnn-example profile list
./build/psnn --root /tmp/psnn-example session demo
./build/psnn --root /tmp/psnn-example diag
./build/psnn --root /tmp/psnn-example diag --json
```

Avoid placing demo profiles in a location that already has real user data;
`init` will refuse to overwrite corrupt or incompatible existing records.
For eventual PS4 userland integration, the intended storage root is
`/data/PSNN`, but access is execution-context-dependent and **untested**.
No background service, network calls, or auto-updater are included.

## Modules

| Path | Purpose | State |
|---|---|---|
| `include/psnn/core.hpp` / `src/core.cpp` | Offline core | Host-tested |
| `include/psnn/adapter_api.h` | Game adapter C ABI v1 | Defined |
| `src/main.cpp` | Local diagnostic CLI | Host-tested |
| `ps4/plugin_entry.c` | Lifecycle/ABI shim | Host syntax-test only |
| `ps4/README.md` | Future OpenOrbis integration | Documented |
| `docs/specs/...` | Milestone 1 design | Documented |
| `tests/test_core.cpp` | Persistence/failure-path regression tests | Host-tested |

## Data and security policy

- Explicit initialization only, never automatic file creation at plugin load.
- Two flat versioned data files, atomic replace and size limits.
- Local profile names restricted to ASCII letters, digits, spaces, dots,
  dashes and underscores; maximum 24 bytes; maximum 64 profiles.
- No external SDK code bundled; use official upstream source under its license.
- No game injection, system-wide hooking, kernel patches, or privilege changes.
- No sensitive secrets, account tokens, PSN credentials or analytics collected.

## Milestone distinction

**Completed now:** offline profile management, atomic persistence, local
session state, JSON/text diagnostics, C adapter ABI, unit and smoke tests.

**Not completed:** real PS4-compatible PRX or executable, verified PS4 storage
access, per-title game adapters, PSN authentication, game-service networking,
LAN multiplayer, friends, achievements, game-mode backend compatibility.

See `docs/VALIDATION.md` for precise observed test outcomes.

### Known limits of v0.1.0

- File persistence is single-writer only; do not run concurrent PSNN instances
  against one storage root until process-level locking is implemented.
- The current CLI is a host/Linux executable. Console toolchain linking and
  file-system behavior need genuine console validation.
- The plugin shim always reports `local_session=0` because it is intentionally
  not connected to the CLI core; the two are separate milestones.

## Join development

Open an issue using the [bug](.github/ISSUE_TEMPLATE/bug_report.yml),
[feature](.github/ISSUE_TEMPLATE/feature_request.yml), or
[PS4 hardware report](.github/ISSUE_TEMPLATE/hardware_report.yml) template.
Please read [CONTRIBUTING.md](CONTRIBUTING.md) for branch, test and review
expectations, [SECURITY.md](SECURITY.md) for sensitive-report handling,
and [RELEASE_PROCESS](docs/RELEASE_PROCESS.md) for console evidence gates.

**Workstream labels to create on GitHub:** `good first issue`, `help wanted`,
`core`, `ps4`, `diagnostics`, `adapters`, `documentation`, `hardware`.

**Licensing:** The project owner has not yet selected an open-source license;
code is publicly visible but use, redistribution, and modification rights
are not granted beyond GitHub platform terms until a license is added.
MIT is proposed and awaiting explicit approval. PSNN is unaffiliated with Sony.
