# FCL Runtime, Network And App Port v1

## Scope

This iteration ports neutral runtime libraries from the downstream reference repository into FCL:

- `asio` as `fcl_asio`;
- `http` as `fcl_http`;
- `websocket` as `fcl_websocket`;
- `quic` as `fcl_quic`;
- `p2p` as `fcl_p2p`;
- `app` as `fcl_app`.

Work happens only in the standalone `storlane-fc` repository. The downstream superproject and submodule pointer are not changed in this pass.

## Network Shape

The network family is not an umbrella library. FCL uses independent root libraries with explicit dependency edges:

- `fcl_websocket -> fcl_http`;
- `fcl_p2p -> fcl_quic`;
- `fcl_quic -> fcl_asio`;
- `fcl_http -> fcl_asio`.

Forbidden names and layouts:

- `libraries/network`;
- legacy network umbrella target;
- legacy net-prefixed targets;
- legacy net-prefixed module names;
- legacy net namespace.

## Accepted Porting Rules

- Public APIs live in real module interface files under `libraries/<lib>/include/fcl/<lib>/*.cppm`.
- Public module files contain declarations directly; they are not wrappers over hidden public headers.
- Implementation files live directly in the library root.
- Product-specific namespaces, module names and include paths are removed.
- QUIC and P2P are mandatory in this pass; CMake requires OpenSSL SSL/Crypto and ngtcp2.
- `Boost.Program_options` stays in `fcl_app` temporarily to preserve the existing plugin API while the runtime port is stabilized.

## Known Cleanup Debt

- The temporary shared BMI producer target has been removed. Each domain target owns its `.cppm` files; future dependency cycles must be fixed in the domain graph instead of hidden behind a module bridge.
- `Boost.Program_options` in `fcl_app` is transitional. A future pass should move CLI/config parsing into an optional adapter.

## Tests

Required tests for this pass:

- `test_fcl_asio`: scheduler order, delay, cancellation, bounded queue and shutdown;
- `test_fcl_app`: port registry, event bus, plugin ordering, rollback and diagnostics;
- `test_fcl_http_websocket`: base URL, target parsing, routing, client/server and TLS WebSocket;
- `test_fcl_quic_p2p`: QUIC handshake/frames/errors and P2P direct/protocol/relay cases.

Static gates must prove that no downstream product namespace, umbrella network library, nested public include directory or broad export wrapper was introduced.
