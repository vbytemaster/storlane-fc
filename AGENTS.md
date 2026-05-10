# AGENTS.md — FCL Engineering Rules

## Role

This repository is evolving into FCL: Foundation Core Libraries for modern C++ products.

The repository must stay neutral. Public APIs must not contain downstream product vocabulary or assumptions.

## Language And Toolchain

- Target language: C++23.
- Public API style: C++ modules first.
- Public module files live under `libraries/<lib>/include/fcl/<lib>/.../*.cppm`.
- `.hpp` files are allowed only for macros, compatibility glue, C/platform shims, and legacy compatibility.
- Do not use `import std;` until the supported toolchain and CI explicitly prove it is stable.

## Library Shape

- Prefer many small targets over one monolithic target.
- Each major layer must become its own target, for example:
  - `fcl_core`
  - `fcl_reflect`
  - `fcl_raw`
  - `fcl_yaml`
  - `fcl_json`
  - `fcl_crypto`
  - `fcl_runtime`
  - `fcl_log`
  - `fcl_config`
  - `fcl_app`
  - `fcl_net_http`
  - `fcl_net_websocket`
  - `fcl_net_quic`
  - `fcl_net_p2p`
  - `fcl_tui`
  - `fcl_fc_compat`
- Heavy classes that own sockets, event loops, crypto contexts, terminal state, or other external resources should use pimpl.
- Value types, protocol records, and simple POD-like structs should not use pimpl.

## Reflection And Serialization

- New canonical reflection uses Boost.Describe or thin FCL wrappers over it.
- Legacy `FC_REFLECT` remains compatibility only.
- Do not map `FC_REFLECT` directly to Boost.Describe with a broad macro alias.
- Binary serialization compatibility with `fc::raw::pack` is a hard gate.
- Any replacement for raw serialization must prove byte-for-byte compatibility with golden tests.
- Reflection field order must be explicit and stable.

## YAML, JSON, And Schema

- YAML and JSON APIs must use described types plus an FCL-owned schema layer.
- Backend parser types must not leak through public module interfaces.
- Validation belongs in schema rules, not in ad hoc parser code.
- Diagnostics must include clear paths, field names, and expected values.
- Secret-like fields must support redaction in configs, logs, diagnostics, and error context.

## Errors And Logging

- New error APIs should be `std`-based and support structured context.
- Legacy `fc::exception` and `FC_CAPTURE_*` remain compatibility only.
- New context capture must support source location and redaction.
- Logging core should stay small: console/file/JSONL-style sinks and structured fields.
- External logging integrations must be optional adapters, not core dependencies.
- Do not log secrets, passphrases, private keys, token values, or raw key material.

## Crypto

- OpenSSL 3+ is the crypto backend baseline.
- There must be one OpenSSL implementation selected in the build graph.
- Do not add BoringSSL.
- Do not shell out to an external `openssl` binary for key or certificate generation.
- Specialized crypto libraries may remain optional targets when they have clear tests and isolated dependencies.
- K1 compatibility must not be replaced with generic OpenSSL ECDSA behavior.

## Runtime And Networking

- Async APIs for heavy operations should use `boost::asio::awaitable<T>`.
- Synchronous wrappers are allowed, but must not be the only API for heavy operations.
- Boost.Asio and Boost.Beast are valid dependencies for future runtime and network targets.
- Legacy networking code from the old compatibility target must not define the new network API.
- Runtime workers must have explicit cancellation, bounded queues where needed, and deterministic shutdown.
- Do not introduce `std::async`, ad hoc polling loops, or unmanaged background threads as core runtime behavior.

## App And Plugins

- The app layer is optional, not a mandatory framework.
- Ports define contracts.
- Plugins orchestrate behavior.
- Adapters connect concrete backends and external systems.
- Events may support diagnostics, but must not become hidden business-flow coupling.
- Lifecycle order and reverse shutdown order must be testable.

## TUI

- Terminal UI primitives must be neutral and reusable.
- Backend terminal library types must not leak from public APIs.
- Rendering and headless tests must enforce redaction.
- UI is not a security boundary.

## Dependencies

- Keep dependencies explicit and target-scoped.
- Optional features must be behind CMake options.
- Avoid dependencies in the compatibility target if they are only needed by future FCL targets.
- Donor or reference code must not become a build dependency unless explicitly accepted as a dependency.

## Tests

- Every compatibility layer needs golden tests.
- Required test groups include raw serialization, reflection, YAML/JSON/schema, crypto, runtime, network, app lifecycle, logging, and TUI.
- Failing compatibility tests are blockers.
- Do not delete crypto tests during non-crypto cleanup.
- Static checks should enforce that removed legacy zones do not reappear.

## Current Iteration Guardrails

- Keep the repository name and legacy compatibility target unchanged until the structure migration is planned.
- Do not introduce Boost.Describe rewrites before the compatibility test baseline exists.
- Do not move neutral libraries from downstream projects until the FCL target structure exists.
- Do not remove crypto primitives in the first pruning pass.
