module;
#include <fcl/exception/macros.hpp>
#include <cstdint>
#include <exception>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

module fcl.crypto.public_key;

import fcl.core.utility;
import fcl.crypto.base58;
import fcl.crypto.common;
import fcl.crypto.sha256;
import fcl.crypto.signature;
import fcl.exception.exception;
import fcl.raw.raw;
import fcl.variant.static_variant;
import fcl.variant;

namespace fcl::crypto {

struct recovery_visitor : fcl::visitor<public_key::storage_type> {
   recovery_visitor(const sha256& digest, bool check_canonical) : _digest(digest), _check_canonical(check_canonical) {}

   template <typename SignatureType> public_key::storage_type operator()(const SignatureType& s) const {
      return public_key::storage_type(s.recover(_digest, _check_canonical));
   }

   const sha256& _digest;
   bool _check_canonical;
};

public_key::public_key(const signature& c, const sha256& digest, bool check_canonical)
    : _storage(std::visit(recovery_visitor(digest, check_canonical), c.storage())) {}

size_t public_key::which() const {
   return _storage.index();
}

static public_key::storage_type parse_base58(const std::string& base58str) {
   constexpr auto legacy_prefix = config::public_key_legacy_prefix;
   if (prefix_matches(legacy_prefix, base58str) && base58str.find('_') == std::string::npos) {
      auto sub_str = base58str.substr(const_strlen(legacy_prefix));
      using default_type = typename std::variant_alternative_t<0, public_key::storage_type>; // public_key::storage_type::template
                                                                                             // type_at<0>;
      using data_type = default_type::data_type;
      using wrapper = checksummed_data<data_type>;
      auto bin = fcl::from_base58(sub_str);
      FCL_ASSERT(bin.size() == sizeof(data_type) + sizeof(uint32_t), "");
      auto wrapped = fcl::raw::unpack<wrapper>(bin);
      FCL_ASSERT(wrapper::calculate_checksum(wrapped.data) == wrapped.check);
      return public_key::storage_type(default_type(wrapped.data));
   } else {
      constexpr auto prefix = config::public_key_base_prefix;

      const auto pivot = base58str.find('_');
      FCL_ASSERT(pivot != std::string::npos, "No delimiter in string, cannot determine data type: ${str}",
                 fcl::error::ctx("str", base58str));

      const auto prefix_str = base58str.substr(0, pivot);
      FCL_ASSERT(prefix == prefix_str, "Public Key has invalid prefix", fcl::error::ctx("str", base58str),
                 fcl::error::ctx("prefix_str", prefix_str));

      auto data_str = base58str.substr(pivot + 1);
      FCL_ASSERT(!data_str.empty(), "Public Key has no data: ${str}", fcl::error::ctx("str", base58str));
      return base58_str_parser<public_key::storage_type, config::public_key_prefix>::apply(data_str);
   }
}

public_key::public_key(const std::string& base58str) : _storage(parse_base58(base58str)) {}

struct is_valid_visitor : public fcl::visitor<bool> {
   template <typename KeyType> bool operator()(const KeyType& key) const {
      return key.valid();
   }
};

bool public_key::valid() const {
   return std::visit(is_valid_visitor(), _storage);
}

std::string public_key::to_string(const fcl::yield_function_t& yield) const {
   auto data_str = std::visit(base58str_visitor<storage_type, config::public_key_prefix, 0>(yield), _storage);

   auto which = _storage.index();
   if (which == 0) {
      return std::string(config::public_key_legacy_prefix) + data_str;
   } else {
      return std::string(config::public_key_base_prefix) + "_" + data_str;
   }
}

std::ostream& operator<<(std::ostream& s, const public_key& k) {
   s << "public_key(" << k.to_string({}) << ')';
   return s;
}

bool operator==(const public_key& p1, const public_key& p2) {
   return eq_comparator<public_key::storage_type>::apply(p1._storage, p2._storage);
}

bool operator!=(const public_key& p1, const public_key& p2) {
   return !(p1 == p2);
}

bool operator<(const public_key& p1, const public_key& p2) {
   return less_comparator<public_key::storage_type>::apply(p1._storage, p2._storage);
}
} // namespace fcl::crypto

namespace fcl {
using namespace std;
void to_variant(const fcl::crypto::public_key& var, fcl::variant& vo, const fcl::yield_function_t& yield) {
   vo = var.to_string(yield);
}

void from_variant(const fcl::variant& var, fcl::crypto::public_key& vo) {
   vo = fcl::crypto::public_key(var.as_string());
}
} // namespace fcl
