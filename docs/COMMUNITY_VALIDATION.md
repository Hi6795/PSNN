# PSNN community source verification — 2026-10-11

Environment: Linux x86-64; no PS4 SDK/toolchain or console attached.

Commands executed on this community tree:
- `bash scripts/verify.sh` — GCC/CMake host build, 2/2 CTest checks passed, CLI smoke passed.
- `cmake -S . -B build-clang -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++` — configured successfully.
- `cmake --build build-clang -j2` — Clang build completed successfully.
- `ctest --test-dir build-clang --output-on-failure` — 2/2 passed.
- YAML files in `.github/` parsed successfully with PyYAML. This does not independently prove GitHub Actions execution.

No genuine PS4 PRX built, no PS4 firmware tested, no per-game adapters integrated. Publishing the files to GitHub and running GitHub-hosted CI are not yet verified.
