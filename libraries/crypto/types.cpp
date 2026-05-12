module;

#include <string>
#include <utility>

module fcl.crypto.types;

namespace fcl::crypto {

error::error(error_kind kind, std::string message) : std::runtime_error(std::move(message)), _kind(kind) {}

error_kind error::kind() const noexcept {
   return _kind;
}

} // namespace fcl::crypto
