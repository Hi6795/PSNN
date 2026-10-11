# PSNN roadmap and collaboration lanes

## Stage 0 — available now: portable offline core v0.1.0

Host/Linux C++11 profile persistence, ephemeral local sessions, JSON/text
storage diagnostics, C adapter ABI definition and a host-only shim.

## Stage 1 — community development (open contribution lanes)

- **Core & reliability:** negative tests, concurrency/lock design,
  storage-directory durability and data format evolution.
- **Build & quality:** CI, sanitizers, portable ABI layout tests, documentation.
- **PS4 integration:** OpenOrbis/GoldHEN source-only build integration,
  diagnostics-only PRX load/unload validation on 11.02 and 13.52 separately.
- **Developer tools:** redacted diagnostics export, capability reports,
  opt-in console-facing interface proposal.

## Stage 2 — after console foundation verified

- Per-game **read-only** compatibility probes and explicit title/version lists.
- Opt-in local profile UI and diagnostics panel.
- Per-game adapter loader whose default is disabled/fail closed.

## Stage 3 — independently specified future modules

- Private LAN discovery and sessions with explicit user consent.
- Optional game-specific services, limited to independently developed local
  behavior and verified compatibility.
- Separate achievement/friend-like features with no claim to Sony PSN state.

## What is not promised

No access to Sony PSN, PS Store, subscriptions, online entitlements, trophy
synchronization, official matchmaking, or every game's online-only backend.
These are not equivalent to creating local PSNN sessions.

## Suggested first contributor issues

1. **good first issue · tests:** add profile validation boundary cases and
   assert failed additions leave on-disk records unchanged.
2. **good first issue · docs:** document exact ABI v1 struct fields, error codes,
   and module lifecycle with host test references.
3. **help wanted · reliability:** propose and test file-locking strategy without
   changing existing single-writer semantics prematurely.
4. **help wanted · PS4:** document reproducible OpenOrbis + GoldHEN build
   integration and a console validation checklist, **without claiming a PRX
   until one has actually been compiled and inspected**.
5. **good first issue · CI:** add a sanitizer-only Linux job and triage any
   findings without weakening compiler/test gates.

Every issue should identify one owner and an independently verifiable outcome.
