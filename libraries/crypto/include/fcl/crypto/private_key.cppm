module;
#include <variant>
#include <boost/describe.hpp>

export module fcl.crypto.private_key;

import fcl.crypto.elliptic;
import fcl.crypto.elliptic_r1;
import fcl.crypto.public_key;
import fcl.crypto.signature;
import fcl.crypto.sha256;
import fcl.crypto.sha512;
import fcl.reflect.reflect;
import fcl.variant.described;
import fcl.variant.static_variant;
import fcl.core.utility;
import fcl.variant;


export namespace fcl::crypto {

   namespace config {
      constexpr const char* private_key_base_prefix = "PVT";
      constexpr const char* private_key_prefix[] = {
         "K1",
         "R1"
      };
   };

   class private_key
   {
      public:
         using storage_type = std::variant<ecc::private_key_shim, r1::private_key_shim>;

         private_key() = default;
         private_key( private_key&& ) = default;
         private_key( const private_key& ) = default;
         private_key& operator=(const private_key& ) = default;

         public_key     get_public_key() const;
         signature      sign( const sha256& digest, bool require_canonical = true ) const;
         sha512         generate_shared_secret( const public_key& pub ) const;

         template< typename KeyType = ecc::private_key_shim >
         static private_key generate() {
            return private_key(storage_type(KeyType::generate()));
         }

         template< typename KeyType = r1::private_key_shim >
         static private_key generate_r1() {
            return private_key(storage_type(KeyType::generate()));
         }

         template< typename KeyType = ecc::private_key_shim >
         static private_key regenerate( const typename KeyType::data_type& data ) {
            return private_key(storage_type(KeyType(data)));
         }

         // serialize to/from string
         explicit private_key(const std::string& base58str);
         std::string to_string(const fcl::yield_function_t& yield) const;

      private:
         storage_type _storage;
         BOOST_DESCRIBE_CLASS(private_key, (), (), (), (_storage))

         private_key( storage_type&& other_storage )
            :_storage(std::move(other_storage))
         {}

         friend bool operator==( const private_key& p1, const private_key& p2 );
         friend bool operator<( const private_key& p1, const private_key& p2 );
   }; // private_key

} // namespace fcl::crypto

export namespace fcl {
   void to_variant(const crypto::private_key& var, variant& vo, const fcl::yield_function_t& yield = fcl::yield_function_t());

   void from_variant(const variant& var, crypto::private_key& vo);
} // namespace fcl

