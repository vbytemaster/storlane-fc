module;

#include <chrono>
#include <memory>
#include <optional>
#include <vector>

export module fcl.p2p.peer_store;

import fcl.p2p.identity;
import fcl.p2p.protocol;
import fcl.p2p.scoring;
import fcl.quic.endpoint;

export namespace fcl::p2p {

struct peer_endpoint_record {
   fcl::quic::endpoint endpoint;
   path_kind kind = path_kind::direct;
   std::optional<peer_id> relay_peer;
   std::uint64_t successes = 0;
   std::uint64_t failures = 0;
   std::chrono::milliseconds last_latency{0};
   std::chrono::steady_clock::time_point backoff_until{};
   double score = 0.0;
};

struct peer_record {
   peer_id peer;
   capability_set capabilities{};
   std::vector<peer_endpoint_record> endpoints;
   reachability_state reachability = reachability_state::unknown;
   std::optional<fcl::quic::endpoint> observed_endpoint;
   std::chrono::steady_clock::time_point reachability_expires_at{};
   std::uint64_t successes = 0;
   std::uint64_t failures = 0;
   std::chrono::milliseconds last_latency{0};
   double score = 0.0;
};

class peer_store {
public:
   peer_store();
   ~peer_store();

   peer_store(const peer_store&) = delete;
   peer_store& operator=(const peer_store&) = delete;

   peer_store(peer_store&&) noexcept;
   peer_store& operator=(peer_store&&) noexcept;

   void upsert(peer_record record);
   void learn_endpoint(peer_id peer, fcl::quic::endpoint endpoint, capability_set capabilities = {});
   void mark_reachability(peer_id peer, reachability_state state, std::optional<fcl::quic::endpoint> observed = std::nullopt);
   void mark_success(const peer_id& peer, path_kind kind, std::chrono::milliseconds latency);
   void mark_failure(const peer_id& peer);
   void mark_endpoint_success(
      const peer_id& peer,
      const fcl::quic::endpoint& endpoint,
      path_kind kind,
      std::chrono::milliseconds latency);
   void mark_endpoint_failure(
      const peer_id& peer,
      const fcl::quic::endpoint& endpoint,
      path_kind kind,
      std::chrono::steady_clock::time_point backoff_until);

   [[nodiscard]] std::optional<peer_record> find(const peer_id& peer) const;
   [[nodiscard]] std::vector<peer_record> snapshot() const;

private:
   struct impl;
   std::shared_ptr<impl> impl_;
};

} // namespace fcl::p2p
