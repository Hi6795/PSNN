# Contributing to PSNN

Thanks for helping develop independent PS4 homebrew. Contributions are welcome
for the portable offline core, documentation, tests, diagnostics and *opt-in*
per-title PS4 adapters. Read [ARCHITECTURE](docs/ARCHITECTURE.md),
[ROADMAP](docs/ROADMAP.md), and [SECURITY](SECURITY.md) before opening a PR.

## Development setup (Linux host)

Requirements: GCC or Clang with C++11 support, CMake >= 3.10, Python 3,
Bash, and Git. PS4 hardware and its development toolchain are **not** needed
for portable-core contributions.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
ctest --test-dir build --output-on-failure
bash scripts/smoke.sh
```

For a single command: `bash scripts/verify.sh`. It builds into `build/`
and runs the host tests. For PS4-adapter contributions, describe **separately**
which title ID, game version, console firmware and loader were actually tested.
Never infer PS4 compatibility from a Linux `.so` or an object-file compile.

## Pull request workflow

1. Search existing issues and pick one scoped deliverable; if untracked,
   open an issue proposing its public interface and test approach.
2. Fork, create a focused feature branch, and avoid unrelated refactors.
3. Include tests for changed behavior and update docs for changed contracts.
4. Run `bash scripts/verify.sh` and attach the command output or CI link.
5. Use the PR template, describing scope, rollback, hardware tests (if any),
   and any files or APIs the change accesses.
6. A reviewer should approve changes before maintainers merge. Failing CI
   or unknown console safety means **do not merge as a hardware-ready change**.

## Contribution boundaries

- Do not check in proprietary Sony SDKs, copyrighted game binaries/assets,
  Sony credentials, account tokens, console IDs, or private log fields.
- Do not imply Sony sign-in success: `local_session` is local-only and
  `network_enabled=0` under adapter ABI v1.
- No stealth hooks, privileged patches, DRM bypass, automatic telemetry,
  or unreviewed runtime downloaders. Firmware/game adapter code must be opt-in.
- Preserve v0.1.0's explicit initialization and fail-closed storage policy.
- ABI changes need a new version rather than silently altering v1 struct layout.
- For uncertain PS4 compatibility, contribute probes or documentation first;
  do not announce hardware support without test evidence.

## Coding conventions

Keep C ABI types in `include/psnn/adapter_api.h`; keep portable logic separate
from `ps4/`; prefer C++11 for core modules and C for entrypoint shims.
Return explicit errors, validate untrusted storage, and keep patches reviewable.
Discussion should remain respectful and technical; see [CODE_OF_CONDUCT](CODE_OF_CONDUCT.md).
