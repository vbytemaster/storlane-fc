module;

#include <cstdint>
#include <string>
#include <vector>

export module fcl.p2p.message;

import fcl.p2p.identity;
import fcl.p2p.protocol;
import fcl.quic.endpoint;

export namespace fcl::p2p {

inline constexpr std::uint16_t wire_version_v1 = 1;
inline constexpr std::uint32_t mandatory_flag_mask = 0x8000'0000U;

enum class message_type : std::uint16_t {
   hello = 1,
   hello_ack = 2,
   protocol_open = 3,
   protocol_accept = 4,
   protocol_reject = 5,
   peer_exchange_request = 6,
   peer_exchange_response = 7,
   relay_open = 8,
   relay_accept = 9,
   relay_reject = 10,
   relay_close = 11,
   ping = 12,
   pong = 13,
   goaway = 14,
   reachability_probe = 15,
   reachability_result = 16,
   relay_reserve = 17,
   relay_renew = 18,
   relay_cancel = 19,
   relay_reserved = 20,
   hole_punch_prepare = 21,
   hole_punch_sync = 22,
   hole_punch_result = 23
};

struct endpoint_record {
   peer_id peer;
   fcl::quic::endpoint endpoint;
   capability_set capabilities{};
};

struct p2p_message {
   message_type type = message_type::ping;
   std::uint64_t request_id = 0;
   std::uint32_t flags = 0;
   peer_id peer;
   protocol_id protocol;
   capability_set capabilities{};
   std::uint64_t max_frame_size = 16 * 1024 * 1024;
   std::uint64_t reservation_id = 0;
   std::uint64_t ttl_ms = 0;
   std::uint64_t max_streams = 0;
   std::uint64_t max_bytes = 0;
   std::uint64_t max_queued_bytes = 0;
   reachability_state reachability = reachability_state::unknown;
   hole_punch_status hole_punch = hole_punch_status::not_attempted;
   std::string reason;
   std::vector<endpoint_record> endpoints;
   std::vector<std::uint8_t> payload;
};

} // namespace fcl::p2p
