module;

#include <compare>
#include <string>
#include <string_view>

export module fcl.p2p.identity;

import fcl.p2p.errors;
import fcl.quic.security;

export namespace fcl::p2p {

struct peer_id {
   std::string value;

   [[nodiscard]] friend bool operator==(const peer_id&, const peer_id&) noexcept = default;
   [[nodiscard]] friend auto operator<=>(const peer_id&, const peer_id&) noexcept = default;
};

[[nodiscard]] peer_id make_peer_id_from_certificate_pem(std::string_view certificate_pem);
[[nodiscard]] peer_id make_peer_id_from_certificate(const fcl::quic::peer_certificate& certificate);
[[nodiscard]] bool valid_peer_id(const peer_id& id) noexcept;

} // namespace fcl::p2p
