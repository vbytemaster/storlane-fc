# FCL Prune FC v1

## Summary

This pass starts the transition from the old FC compatibility codebase toward FCL.

The repository and legacy target names stay unchanged in this pass. The goal is to remove legacy non-crypto code that should not become part of FCL core, while preserving the compatibility areas needed for raw serialization, variant/json, reflection, exceptions, time, and crypto.

Crypto is intentionally preserved. Network code from the old FC layer is intentionally removed; future network support belongs in dedicated FCL targets.

## Keep

- All current crypto code and tests: BLS12-381, BN256, GMP-backed modular arithmetic, K1/secp256k1, R1/WebAuthn, AES, hashes, random, base58/base64/hex, key and signature wrappers.
- `fc::raw::pack/unpack`, datastream, varint, and raw compatibility tests.
- `fc::variant`, `fc::variant_object`, `fc::json`, reflection compatibility, and related tests.
- `fc::exception`, `FC_CAPTURE_*`, log message, logger, and the minimal console appender path needed for compatibility diagnostics.
- `fc::time_point`, `fc::time_point_sec`, string, utf8, filesystem, static variant, and scalar utilities.
- Boost.Asio and Boost.Beast remain allowed dependencies for future FCL runtime/network targets, even if they are removed from the legacy `fc` target linkage.

## Remove In This Pass

- Legacy FC network layer:
  - `include/fc/network/*`
  - `src/network/*`
  - `test/network/*`
  - network source/test entries in CMake.
- Legacy logging integrations:
  - GELF appender
  - DMLOG appender
  - zlib compression helper used by GELF
  - ZLIB linkage when no remaining source needs it.
- Old persistence and interprocess helpers:
  - `random_access_file`
  - `persistence_util`
  - `cfile`
  - `io/console`
  - `interprocess/file_mapping`
  - related tests.
- Old standalone containers without an FCL v1 role:
  - `tracked_storage`
  - `ordered_diff`
  - related tests.

## Not In This Pass

- No Boost.Describe rewrite.
- No `FC_REFLECT` migration.
- No repository rename.
- No target rename from `fc` to `fcl_*`.
- No new `libraries/<lib>/include/fcl/...` structure.
- No downstream library import.
- No crypto pruning.

## Validation

Required build and tests:

```sh
cmake -S . -B build/fcl-prune-debug -G Ninja -DBUILD_TESTING=ON
cmake --build build/fcl-prune-debug -j 1 --target fc test_fc
ctest --test-dir build/fcl-prune-debug --output-on-failure --timeout 180
```

Required static checks:

```sh
rg "fc/network|http_client|message_buffer|network/listener|network/url" include src test CMakeLists.txt
rg "gelf_appender|dmlog_appender|compress/zlib|ZLIB|tracked_storage|ordered_diff|random_access_file|persistence_util|file_mapping" include src test CMakeLists.txt
rg "storlane|spring|workspace|mountd|contentd|repaird|grant|acl|provider|obligation|receipt" AGENTS.md
git diff --check
```

The first two `rg` commands must be empty for removed legacy zones.

## Follow-Up Direction

Next passes should introduce the FCL target structure, then create the reflection/raw compatibility baseline before replacing legacy reflection internals.
