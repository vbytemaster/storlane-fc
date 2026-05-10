#pragma once
#include <fcl/variant/static_variant.hpp>
#include <fcl/crypto/elliptic.hpp>
#include <fcl/crypto/elliptic_r1.hpp>
#include <fcl/crypto/elliptic_webauthn.hpp>
#include <fcl/reflect/reflect.hpp>
#include <fcl/reflect/variant.hpp>

namespace fcl { namespace crypto {
   namespace config {
      constexpr const char* signature_base_prefix = "SIG";
      constexpr const char* signature_prefix[] = {
         "K1",
         "R1",
         "WA"
      };
   };

   class signature
   {
      public:
         using storage_type = std::variant<ecc::signature_shim, r1::signature_shim, webauthn::signature>;

         signature() = default;
         signature( signature&& ) = default;
         signature( const signature& ) = default;
         signature& operator= (const signature& ) = default;

         // serialize to/from string
         explicit signature(const std::string& base58str);
         std::string to_string(const fcl::yield_function_t& yield = fcl::yield_function_t()) const;

         constexpr bool is_webauthn() const { return _storage.index() == fcl::get_index<storage_type, webauthn::signature>(); }

         size_t which() const;

         size_t variable_size() const;

      private:
         storage_type _storage;

         signature( storage_type&& other_storage )
         :_storage(std::move(other_storage))
         {}

         friend bool operator == ( const signature& p1, const signature& p2);
         friend bool operator != ( const signature& p1, const signature& p2);
         friend bool operator < ( const signature& p1, const signature& p2);
         friend std::size_t hash_value(const signature& b); //not cryptographic; for containers
         friend struct reflector<signature>;
         friend class private_key;
         friend class public_key;
   }; // public_key

   size_t hash_value(const signature& b);

} }  // fcl::crypto

namespace fcl {
   void to_variant(const crypto::signature& var, variant& vo, const fcl::yield_function_t& yield = fcl::yield_function_t());

   void from_variant(const variant& var, crypto::signature& vo);
} // namespace fcl

namespace std {
   template <> struct hash<fcl::crypto::signature> {
      std::size_t operator()(const fcl::crypto::signature& k) const {
         return fcl::crypto::hash_value(k);
      }
   };
} // std

FCL_REFLECT(fcl::crypto::signature, (_storage) )
