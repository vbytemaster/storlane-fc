module;
#include <variant>
#include <boost/describe.hpp>

export module fcl.crypto.signature;

import fcl.variant.static_variant;
import fcl.crypto.elliptic;
import fcl.crypto.elliptic_r1;
import fcl.crypto.elliptic_webauthn;
import fcl.reflect.reflect;
import fcl.variant.described;
import fcl.core.utility;
import fcl.variant;

export namespace fcl::crypto {
namespace config {
constexpr const char* signature_base_prefix = "SIG";
constexpr const char* signature_prefix[] = {"K1", "R1", "WA"};
}; // namespace config

class signature {
 public:
   using storage_type = std::variant<ecc::signature_shim, r1::signature_shim, webauthn::signature>;

   signature() = default;
   signature(signature&&) = default;
   signature(const signature&) = default;
   signature& operator=(const signature&) = default;

   // serialize to/from string
   explicit signature(const std::string& base58str);
   std::string to_string(const fcl::yield_function_t& yield = fcl::yield_function_t()) const;

   constexpr bool is_webauthn() const {
      return _storage.index() == fcl::get_index<storage_type, webauthn::signature>();
   }

   size_t which() const;

   size_t variable_size() const;
   const storage_type& storage() const {
      return _storage;
   }

   explicit signature(storage_type&& other_storage) : _storage(std::move(other_storage)) {}

 private:
   storage_type _storage;
   BOOST_DESCRIBE_CLASS(signature, (), (), (), (_storage))

   friend bool operator==(const signature& p1, const signature& p2);
   friend bool operator!=(const signature& p1, const signature& p2);
   friend bool operator<(const signature& p1, const signature& p2);
   friend std::size_t hash_value(const signature& b); // not cryptographic; for containers
}; // public_key

size_t hash_value(const signature& b);

} // namespace fcl::crypto

export namespace fcl {
void to_variant(const crypto::signature& var, variant& vo,
                const fcl::yield_function_t& yield = fcl::yield_function_t());

void from_variant(const variant& var, crypto::signature& vo);
} // namespace fcl

export namespace std {
template <> struct hash<fcl::crypto::signature> {
   std::size_t operator()(const fcl::crypto::signature& k) const {
      return fcl::crypto::hash_value(k);
   }
};
} // namespace std
