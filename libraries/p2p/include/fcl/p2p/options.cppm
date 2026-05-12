module;

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

export module fcl.p2p.options;

import fcl.p2p.identity;
import fcl.p2p.protocol;
import fcl.p2p.relay;
import fcl.quic.endpoint;
import fcl.quic.options;

export namespace fcl::p2p {

struct node_limits {
   std::size_t max_sessions = 1024;
   std::size_t max_protocol_handlers = 1024;
   std::size_t max_control_message_size = 4 * 1024 * 1024;
   std::size_t max_peer_exchange_records = 1024;
   std::size_t max_control_queue = 4096;
   relay_limits relay{};
};

struct node_options {
   std::string certificate_pem;
   std::string private_key_pem;
   std::optional<peer_id> explicit_peer_id;
   capability_set capabilities{.bits = capabilities::direct_quic | capabilities::peer_exchange};
   node_limits limits{};
   fcl::quic::transport_limits transport_limits{};
   std::vector<fcl::quic::endpoint> advertised_endpoints;
   bool allow_insecure_test_mode = false;
};

struct connect_options {
   std::optional<peer_id> expected_peer;
   bool allow_relay = true;
   std::optional<peer_id> relay_peer;
   std::chrono::milliseconds timeout{10'000};
   std::chrono::milliseconds direct_attempt_timeout{2'000};
   std::chrono::milliseconds relay_attempt_timeout{5'000};
   std::size_t max_direct_endpoints = 4;
   std::size_t max_relay_candidates = 4;
   bool allow_hole_punch = true;
};

struct open_options {
   bool allow_relay = true;
   std::optional<peer_id> relay_peer;
   std::chrono::milliseconds timeout{10'000};
   std::chrono::milliseconds direct_attempt_timeout{2'000};
   std::chrono::milliseconds relay_attempt_timeout{5'000};
   std::size_t max_direct_endpoints = 4;
   std::size_t max_relay_candidates = 4;
   bool allow_hole_punch = true;
};

void validate(const node_options& options);

} // namespace fcl::p2p
