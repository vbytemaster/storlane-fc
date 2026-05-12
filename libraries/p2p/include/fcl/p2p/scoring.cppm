module;

#include <chrono>
#include <cstdint>

export module fcl.p2p.scoring;

export namespace fcl::p2p {

enum class path_kind { direct, relay };

struct path_observation {
   path_kind kind = path_kind::direct;
   std::chrono::milliseconds latency{0};
   std::uint64_t failures = 0;
   std::uint64_t successes = 0;
   std::uint64_t in_flight = 0;
   bool last_success = false;
};

[[nodiscard]] double score_path(const path_observation& observation) noexcept;

} // namespace fcl::p2p
