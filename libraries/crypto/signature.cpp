module;
#include <fcl/exception/macros.hpp>
#include <exception>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>

module fcl.crypto.signature;

import fcl.core.utility;
import fcl.crypto.common;
import fcl.crypto.elliptic_webauthn;
import fcl.exception.exception;
import fcl.variant.static_variant;
import fcl.variant;

namespace fcl::crypto {
struct hash_visitor : public fcl::visitor<size_t> {
   template <typename SigType> size_t operator()(const SigType& sig) const {
      static_assert(std::tuple_size_v<std::remove_reference_t<decltype(sig._data)>> == 65,
                    "sig size is expected to be 65");
      // signatures are two bignums: r & s. Just add up least significant digits of the two
      return *(size_t*)&sig._data.data()[32 - sizeof(size_t)] + *(size_t*)&sig._data.data()[64 - sizeof(size_t)];
   }

   size_t operator()(const webauthn::signature& sig) const {
      return sig.get_hash();
   }
};

static signature::storage_type sig_parse_base58(const std::string& base58str) {
   try {
      constexpr auto prefix = config::signature_base_prefix;

      const auto pivot = base58str.find('_');
      FCL_ASSERT(pivot != std::string::npos, "No delimiter in string, cannot determine type: ${str}",
                 fcl::error::ctx("str", base58str));

      const auto prefix_str = base58str.substr(0, pivot);
      FCL_ASSERT(prefix == prefix_str, "Signature Key has invalid prefix", fcl::error::ctx("str", base58str),
                 fcl::error::ctx("prefix_str", prefix_str));

      auto data_str = base58str.substr(pivot + 1);
      FCL_ASSERT(!data_str.empty(), "Signature has no data: ${str}", fcl::error::ctx("str", base58str));
      return base58_str_parser<signature::storage_type, config::signature_prefix>::apply(data_str);
   }
   FCL_CAPTURE_AND_RETHROW("error parsing signature", fcl::error::ctx("str", base58str))
}

signature::signature(const std::string& base58str) : _storage(sig_parse_base58(base58str)) {}

size_t signature::which() const {
   return _storage.index();
}

template <class... Ts> struct overloaded : Ts... {
   using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

size_t signature::variable_size() const {
   return std::visit(overloaded{[&](const auto& k1r1) { return static_cast<size_t>(0); },
                                [&](const webauthn::signature& wa) { return static_cast<size_t>(wa.variable_size()); }},
                     _storage);
}

std::string signature::to_string(const fcl::yield_function_t& yield) const {
   auto data_str = std::visit(base58str_visitor<storage_type, config::signature_prefix>(yield), _storage);
   yield();
   return std::string(config::signature_base_prefix) + "_" + data_str;
}

std::ostream& operator<<(std::ostream& s, const signature& k) {
   s << "signature(" << k.to_string() << ')';
   return s;
}

bool operator==(const signature& p1, const signature& p2) {
   return eq_comparator<signature::storage_type>::apply(p1._storage, p2._storage);
}

bool operator!=(const signature& p1, const signature& p2) {
   return !eq_comparator<signature::storage_type>::apply(p1._storage, p2._storage);
}

bool operator<(const signature& p1, const signature& p2) {
   return less_comparator<signature::storage_type>::apply(p1._storage, p2._storage);
}

size_t hash_value(const signature& b) {
   return std::visit(hash_visitor(), b._storage);
}
} // namespace fcl::crypto

namespace fcl {
void to_variant(const fcl::crypto::signature& var, fcl::variant& vo, const fcl::yield_function_t& yield) {
   vo = var.to_string(yield);
}

void from_variant(const fcl::variant& var, fcl::crypto::signature& vo) {
   vo = fcl::crypto::signature(var.as_string());
}
} // namespace fcl
