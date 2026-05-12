module;

#include <stdexcept>
#include <string>

module fcl.quic.errors;

namespace fcl::quic {

const char* to_string(error_kind kind) noexcept {
   switch (kind) {
   case error_kind::invalid_endpoint:
      return "invalid_endpoint";
   case error_kind::invalid_options:
      return "invalid_options";
   case error_kind::dependency_unavailable:
      return "dependency_unavailable";
   case error_kind::connect_timeout:
      return "connect_timeout";
   case error_kind::handshake_timeout:
      return "handshake_timeout";
   case error_kind::idle_timeout:
      return "idle_timeout";
   case error_kind::tls_failed:
      return "tls_failed";
   case error_kind::peer_verification_failed:
      return "peer_verification_failed";
   case error_kind::alpn_mismatch:
      return "alpn_mismatch";
   case error_kind::frame_too_large:
      return "frame_too_large";
   case error_kind::malformed_frame:
      return "malformed_frame";
   case error_kind::backpressure_rejected:
      return "backpressure_rejected";
   case error_kind::connection_closed:
      return "connection_closed";
   case error_kind::stream_closed:
      return "stream_closed";
   case error_kind::stream_reset:
      return "stream_reset";
   case error_kind::canceled:
      return "canceled";
   case error_kind::unsupported:
      return "unsupported";
   case error_kind::internal_error:
      return "internal_error";
   }
   return "unknown";
}

quic_error::quic_error(error_kind kind, std::string message) : std::runtime_error(std::move(message)), kind_(kind) {}

error_kind quic_error::kind() const noexcept {
   return kind_;
}

void throw_quic_error(error_kind kind, std::string message) {
   throw quic_error{kind, std::move(message)};
}

} // namespace fcl::quic
