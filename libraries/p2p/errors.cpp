module;

#include <string>
#include <utility>

module fcl.p2p.errors;

namespace fcl::p2p {

const char* to_string(error_kind kind) noexcept {
   switch (kind) {
   case error_kind::invalid_options:
      return "invalid_options";
   case error_kind::invalid_identity:
      return "invalid_identity";
   case error_kind::protocol_error:
      return "protocol_error";
   case error_kind::codec_error:
      return "codec_error";
   case error_kind::unsupported_protocol:
      return "unsupported_protocol";
   case error_kind::duplicate_protocol:
      return "duplicate_protocol";
   case error_kind::peer_not_found:
      return "peer_not_found";
   case error_kind::peer_verification_failed:
      return "peer_verification_failed";
   case error_kind::relay_not_available:
      return "relay_not_available";
   case error_kind::relay_rejected:
      return "relay_rejected";
   case error_kind::backpressure_rejected:
      return "backpressure_rejected";
   case error_kind::timeout:
      return "timeout";
   case error_kind::canceled:
      return "canceled";
   case error_kind::closed:
      return "closed";
   case error_kind::internal_error:
      return "internal_error";
   }
   return "internal_error";
}

p2p_error::p2p_error(error_kind kind, std::string message) : std::runtime_error(std::move(message)), kind_(kind) {}

error_kind p2p_error::kind() const noexcept {
   return kind_;
}

void throw_p2p_error(error_kind kind, std::string message) {
   throw p2p_error{kind, std::move(message)};
}

} // namespace fcl::p2p
