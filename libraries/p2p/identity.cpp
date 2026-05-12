module;

#include <algorithm>
#include <cctype>
#include <string_view>

module fcl.p2p.identity;

namespace fcl::p2p {

namespace {

[[nodiscard]] bool is_lower_hex_string(std::string_view value) noexcept {
   return std::ranges::all_of(
       value, [](char ch) { return std::isdigit(static_cast<unsigned char>(ch)) != 0 || (ch >= 'a' && ch <= 'f'); });
}

} // namespace

peer_id make_peer_id_from_certificate_pem(std::string_view certificate_pem) {
   auto fingerprint = fcl::quic::certificate_sha256_fingerprint_from_pem(certificate_pem);
   auto id = peer_id{.value = std::move(fingerprint)};
   if (!valid_peer_id(id)) {
      throw_p2p_error(error_kind::invalid_identity, "certificate did not produce a valid P2P peer id");
   }
   return id;
}

peer_id make_peer_id_from_certificate(const fcl::quic::peer_certificate& certificate) {
   auto fingerprint = certificate.sha256_fingerprint.empty()
                          ? fcl::quic::sha256_fingerprint(certificate.der)
                          : fcl::quic::normalize_sha256_fingerprint(certificate.sha256_fingerprint);
   auto id = peer_id{.value = std::move(fingerprint)};
   if (!valid_peer_id(id)) {
      throw_p2p_error(error_kind::invalid_identity, "peer certificate did not produce a valid P2P peer id");
   }
   return id;
}

bool valid_peer_id(const peer_id& id) noexcept {
   return id.value.size() == 64 && is_lower_hex_string(id.value);
}

} // namespace fcl::p2p
