module;

#include <cstddef>
#include <cstdint>

export module fcl.p2p.metrics;

export namespace fcl::p2p {

struct node_metrics {
   std::uint64_t sessions_opened = 0;
   std::uint64_t sessions_closed = 0;
   std::uint64_t handshakes_completed = 0;
   std::uint64_t handshakes_failed = 0;
   std::uint64_t protocol_streams_opened = 0;
   std::uint64_t protocol_streams_accepted = 0;
   std::uint64_t protocol_rejections = 0;
   std::uint64_t peer_exchange_messages = 0;
   std::uint64_t reachability_probes = 0;
   std::uint64_t reachability_public = 0;
   std::uint64_t reachability_private = 0;
   std::uint64_t relays_opened = 0;
   std::uint64_t relay_rejections = 0;
   std::uint64_t relay_reservations = 0;
   std::uint64_t relay_reservation_rejections = 0;
   std::uint64_t relay_reservation_expirations = 0;
   std::uint64_t relay_bytes = 0;
   std::uint64_t hole_punch_attempts = 0;
   std::uint64_t hole_punch_successes = 0;
   std::uint64_t hole_punch_failures = 0;
   std::uint64_t path_direct_opens = 0;
   std::uint64_t path_relay_opens = 0;
   std::uint64_t path_direct_attempts = 0;
   std::uint64_t path_relay_attempts = 0;
   std::uint64_t direct_failures = 0;
   std::uint64_t relay_failures = 0;
   std::uint64_t backpressure_rejections = 0;
   std::size_t active_sessions = 0;
   std::size_t active_relays = 0;
   std::size_t active_relay_reservations = 0;
   bool stopped = false;
};

} // namespace fcl::p2p
