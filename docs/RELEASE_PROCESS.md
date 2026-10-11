# Release and verification policy

PSNN versions are independent of Sony firmware numbers. Use a git tag
`vMAJOR.MINOR.PATCH` for source releases. Labels **host-verified**,
**PS4-build-verified**, and **console-verified** describe separate evidence.

## Host-verified (current level)

Run `bash scripts/verify.sh` from a clean checkout; archive command output,
compiler/CMake versions and test results. Check that source archives exclude
profiles, console dumps, credentials, and generated Linux binaries.

## PS4-build-verified (not yet reached)

Only after compiling with the OpenOrbis/GoldHEN toolchain, record a PRX
artifact, version, SHA-256, build command, import/export inspection, and
supported loader assumptions. A Linux shared library is **not** a PS4 PRX.

## Console-verified (not yet reached)

Record firmware, game title ID, game version, loader and test actions for each
console. First check startup/unload with **no hooks or storage writes**. Check
storage and title adapters independently after the safe lifecycle gate.

| Firmware | Current status |
|---|---|
| 11.02 | Not tested |
| 13.52 | Not tested |

Do not promote a PRX based only on host CI. Do not ship a game adapter if its
firmware/game compatibility is unverified. Stop and document crash evidence.
