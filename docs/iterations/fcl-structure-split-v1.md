# FCL Structure Split v1

## Summary

This pass moves the codebase from the old source layout into domain-owned FCL libraries.

The pass is intentionally source-breaking: public headers move to `libraries/<lib>/include/fcl/...`, namespaces become `fcl`, and CMake targets become `fcl_*`. The raw binary layout remains the compatibility boundary.

## Layout

FCL-owned libraries use this shape:

```text
libraries/<lib>/
  CMakeLists.txt
  include/fcl/<lib>/*.cppm
  include/fcl/<lib>/*.hpp
  *.cpp
  *_private.hpp
  tests/*.cpp
```

Third-party source and submodules live only under root `vendor/`.

## Domain Targets

- `fcl_core`: scalar helpers, time, filesystem, string, utf8, utility helpers.
- `fcl_exception`: exception and capture helpers.
- `fcl_reflect`: temporary textual reflection macros.
- `fcl_variant`: variant, variant object, static variant, dynamic bitset.
- `fcl_io`: datastream, varint, fstream and low-level stream helpers.
- `fcl_raw`: raw pack and unpack templates.
- `fcl_json`: JSON parser/emitter compatibility behavior.
- `fcl_log`: logger, log message and console appender.
- `fcl_crypto`: hashes, AES, K1/R1/WebAuthn/BLS, base encodings, random and key wrappers.
- `fcl`: umbrella static target assembled from the domain targets.

## Non-Goals

- No Boost.Describe rewrite.
- No semantic rewrite of raw serialization.
- No crypto pruning.
- No new network layer.
- No new YAML/schema/logging design.

## Acceptance

- Root `include/`, `src/`, `test/` and old top-level third-party paths are gone from tracked files.
- `libraries/` has no nested `src/` or `vendor/` directories.
- The removed legacy network, GELF, DMLOG, zlib, old persistence and old container zones do not reappear.
- `test_fcl` keeps the old behavioral coverage under the new `fcl` API.
- A small raw golden test proves representative bytes stay stable across the rename.
