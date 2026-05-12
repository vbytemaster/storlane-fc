module;
#include <variant>
#include <boost/describe.hpp>

export module fcl.crypto.public_key;

import fcl.crypto.elliptic;
import fcl.crypto.elliptic_r1;
import fcl.crypto.elliptic_webauthn;
import fcl.crypto.signature;
import fcl.reflect.reflect;
import fcl.variant.described;
import fcl.variant.static_variant;
import fcl.crypto.sha256;
import fcl.core.utility;
import fcl.variant;

export namespace fcl::crypto {
namespace config {
constexpr const char* public_key_legacy_prefix = "EOS";
constexpr const char* public_key_base_prefix = "PUB";
constexpr const char* public_key_prefix[] = {"K1", "R1", "WA"};
}; // namespace config

class public_key {
 public:
   using storage_type = std::variant<ecc::public_key_shim, r1::public_key_shim, webauthn::public_key>;

   public_key() = default;
   public_key(public_key&&) = default;
   public_key(const public_key&) = default;
   public_key& operator=(const public_key&) = default;

   public_key(const signature& c, const sha256& digest, bool check_canonical = true);

   public_key(storage_type&& other_storage) : _storage(std::move(other_storage)) {}

   bool valid() const;

   size_t which() const;

   // serialize to/from string
   explicit public_key(const std::string& base58str);
   std::string to_string(const fcl::yield_function_t& yield) const;

   storage_type _storage;
   BOOST_DESCRIBE_CLASS(public_key, (), (), (), (_storage))

 private:
   friend std::ostream& operator<<(std::ostream& s, const public_key& k);
   friend bool operator==(const public_key& p1, const public_key& p2);
   friend bool operator!=(const public_key& p1, const public_key& p2);
   friend bool operator<(const public_key& p1, const public_key& p2);
}; // public_key

} // namespace fcl::crypto

export namespace fcl {
void to_variant(const crypto::public_key& var, variant& vo,
                const fcl::yield_function_t& yield = fcl::yield_function_t());

void from_variant(const variant& var, crypto::public_key& vo);
} // namespace fcl
