module;

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

export module fcl.crypto.types;

export namespace fcl::crypto {

using bytes = std::vector<std::uint8_t>;

enum class error_kind {
   invalid_key,
   invalid_nonce,
   invalid_tag,
   invalid_options,
   authentication_failed,
   backend_error,
};

class error final : public std::runtime_error {
public:
   error(error_kind kind, std::string message);

   [[nodiscard]] error_kind kind() const noexcept;

private:
   error_kind _kind;
};

} // namespace fcl::crypto
