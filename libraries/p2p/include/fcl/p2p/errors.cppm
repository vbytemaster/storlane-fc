module;

#include <stdexcept>
#include <string>

export module fcl.p2p.errors;

export namespace fcl::p2p {

enum class error_kind {
   invalid_options,
   invalid_identity,
   protocol_error,
   codec_error,
   unsupported_protocol,
   duplicate_protocol,
   peer_not_found,
   peer_verification_failed,
   relay_not_available,
   relay_rejected,
   backpressure_rejected,
   timeout,
   canceled,
   closed,
   internal_error
};

[[nodiscard]] const char* to_string(error_kind kind) noexcept;

class p2p_error final : public std::runtime_error {
 public:
   p2p_error(error_kind kind, std::string message);

   [[nodiscard]] error_kind kind() const noexcept;

 private:
   error_kind kind_;
};

[[noreturn]] void throw_p2p_error(error_kind kind, std::string message);

} // namespace fcl::p2p
