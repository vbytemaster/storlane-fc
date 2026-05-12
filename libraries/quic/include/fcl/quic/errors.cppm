module;

#include <stdexcept>
#include <string>

export module fcl.quic.errors;

export namespace fcl::quic {

enum class error_kind {
   invalid_endpoint,
   invalid_options,
   dependency_unavailable,
   connect_timeout,
   handshake_timeout,
   idle_timeout,
   tls_failed,
   peer_verification_failed,
   alpn_mismatch,
   frame_too_large,
   malformed_frame,
   backpressure_rejected,
   connection_closed,
   stream_closed,
   stream_reset,
   canceled,
   unsupported,
   internal_error
};

[[nodiscard]] const char* to_string(error_kind kind) noexcept;

class quic_error final : public std::runtime_error {
 public:
   quic_error(error_kind kind, std::string message);

   [[nodiscard]] error_kind kind() const noexcept;

 private:
   error_kind kind_;
};

[[noreturn]] void throw_quic_error(error_kind kind, std::string message);

} // namespace fcl::quic
