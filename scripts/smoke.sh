#!/usr/bin/env bash
set -euo pipefail
BASE="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$BASE/build/psnn"
if [[ ! -x "$BIN" ]]; then echo "First run: cmake -S . -B build && cmake --build build" >&2; exit 2; fi
DIR="$(mktemp -d /tmp/psnn-smoke-XXXXXX)"
trap 'rm -f "$DIR/profiles.psnn" "$DIR/active.psnn"; rmdir "$DIR"' EXIT
"$BIN" --root "$DIR" init
"$BIN" --root "$DIR" profile add "Example Player"
"$BIN" --root "$DIR" profile select LOCAL-00000001
"$BIN" --root "$DIR" profile current
"$BIN" --root "$DIR" session demo
"$BIN" --root "$DIR" diag --json | python3 -c '
import json,sys
v=json.load(sys.stdin)
assert v["schema"] == "psnn.diagnostics.v1"
c = {x["name"]: x for x in v["checks"]}
assert c["storage_write_probe"]["status"] == "PASS"
assert c["network"]["status"] == "INFO" and "disabled" in c["network"]["detail"]
assert c["sony_psn_auth"]["status"] == "INFO"
print("SMOKE_PASS diagnostics_json parseable, storage probed, network disabled, Sony auth unavailable")
'
