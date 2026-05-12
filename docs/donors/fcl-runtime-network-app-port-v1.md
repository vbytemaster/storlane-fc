# Donor Traceability: FCL Runtime, Network And App Port v1

## Donor Files

Reference repository files inspected and ported:

- `libraries/asio/include/storlane/asio/*.cppm`
- `libraries/asio/*.cpp`
- `libraries/network/http/include/storlane/network/http/*.cppm`
- `libraries/network/http/*.cpp`
- `libraries/network/websocket/include/storlane/network/websocket/*.cppm`
- `libraries/network/websocket/*.cpp`
- `libraries/network/quic/include/storlane/network/quic/*.cppm`
- `libraries/network/quic/*.cpp`
- `libraries/network/p2p/include/storlane/network/p2p/*.cppm`
- `libraries/network/p2p/*.cpp`
- `libraries/app/include/storlane/app/*.cppm`
- `libraries/app/*.cpp`
- `tests/unit/asio/task_scheduler_tests.cpp`
- `tests/unit/app/app_tests.cpp`
- `tests/unit/network/network_tests.cpp`
- `tests/unit/network/quic_tests.cpp`
- `tests/unit/network/p2p_tests.cpp`

## Accepted Patterns

- Boost.Asio runtime boundary with explicit startup/shutdown and blocking helper.
- Priority task scheduler with bounded queue, cancellation and deterministic shutdown.
- HTTP client/server split with route context and middleware.
- WebSocket as a separate library that depends on HTTP mechanics.
- QUIC as a separate library over `asio`, OpenSSL and ngtcp2.
- P2P as a separate library over QUIC, not as a generic network umbrella.
- App ports/plugins/events/diagnostics as neutral runtime plumbing, not product authority.

## Rejected Patterns

- Downstream product namespace, module name or include path in FCL public API.
- `libraries/network` umbrella layout.
- legacy net-prefixed module, namespace, target or umbrella names.
- Benchmarks in default `ctest`.
- Product protocol names such as `/storlane/...` or `storlane-p2p/1`.
- UI/events/plugin signals as hidden business-flow coupling.

## FCL Target

The port creates neutral libraries:

- `fcl_asio`;
- `fcl_http`;
- `fcl_websocket`;
- `fcl_quic`;
- `fcl_p2p`;
- `fcl_app`.

The target graph is explicit:

- `websocket -> http`;
- `p2p -> quic`;
- `quic -> asio`;
- `http -> asio`.

## Proof

Proof is provided by ported unit tests and static gates:

- `test_fcl_asio`;
- `test_fcl_app`;
- `test_fcl_http_websocket`;
- `test_fcl_quic_p2p`;
- donor namespace grep over `libraries tests`;
- no legacy net-prefix static grep;
- `find libraries -maxdepth 2 -type d -name network -print`.
