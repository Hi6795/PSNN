#!/usr/bin/env bash
# Linux-host verification only. Not a PS4 build, firmware test or release.
set -euo pipefail
BASE="$(cd "$(dirname "$0")/.." && pwd)"
cd "$BASE"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j2
ctest --test-dir build --output-on-failure
bash scripts/smoke.sh
