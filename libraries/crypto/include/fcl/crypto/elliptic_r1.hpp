#pragma once
#include <fcl/crypto/bigint.hpp>
#include <fcl/crypto/common.hpp>
#include <fcl/crypto/sha256.hpp>
#include <fcl/crypto/sha512.hpp>
#include <fcl/crypto/openssl.hpp>
#include <fcl/core/fwd.hpp>
#include <fcl/core/array.hpp>
#include <fcl/raw/raw_fwd.hpp>

namespace fcl {

  namespace crypto { namespace r1 {
    namespace detail
    {
      class public_key_impl;
      class private_key_impl;
    }

    typedef fcl::array<char,33>          public_key_data;
    typedef fcl::sha256                  private_key_secret;
    typedef fcl::array<char,65>          public_key_point_data; ///< the full non-compressed version of the ECC point
    typedef fcl::array<char,72>          signature;
    typedef fcl::array<unsigned char,65> compact_signature;

    public_key_data recover_public_key_data(const compact_signature& c, const fcl::sha256& digest, bool check_canonical = true);

    /**
     *  @class public_key
     *  @brief contains only the public point of an elliptic curve key.
     */
    class public_key
    {
        public:
           public_key();
           public_key(const public_key& k);
           ~public_key();
           bool verify( const fcl::sha256& digest, const signature& sig );
           public_key_data serialize()const;

           operator public_key_data()const { return serialize(); }


           public_key( const public_key_data& v );
           public_key( const public_key_point_data& v );
           public_key( const compact_signature& c, const fcl::sha256& digest, bool check_canonical = true );

           bool valid()const;
           public_key mult( const fcl::sha256& offset );
           public_key add( const fcl::sha256& offset )const;

           public_key( public_key&& pk );
           public_key& operator=( public_key&& pk );
           public_key& operator=( const public_key& pk );

           inline friend bool operator==( const public_key& a, const public_key& b )
           {
            return a.serialize() == b.serialize();
           }
           inline friend bool operator!=( const public_key& a, const public_key& b )
           {
            return a.serialize() != b.serialize();
           }

        private:
          friend class private_key;
          fcl::fwd<detail::public_key_impl,40> my;
    };

    /**
     *  @class private_key
     *  @brief an elliptic curve private key.
     */
    class private_key
    {
        public:
           private_key();
           private_key( private_key&& pk );
           private_key( const private_key& pk );
           ~private_key();

           private_key& operator=( private_key&& pk );
           private_key& operator=( const private_key& pk );

           static private_key generate();
           static private_key regenerate( const fcl::sha256& secret );

           /**
            *  This method of generation enables creating a new private key in a deterministic manner relative to
            *  an initial seed.   A public_key created from the seed can be multiplied by the offset to calculate
            *  the new public key without having to know the private key.
            */
           static private_key generate_from_seed( const fcl::sha256& seed, const fcl::sha256& offset = fcl::sha256() );

           private_key_secret get_secret()const; // get the private key secret

           operator private_key_secret ()const { return get_secret(); }

           /**
            *  Given a public key, calculatse a 512 bit shared secret between that
            *  key and this private key.
            */
           fcl::sha512 get_shared_secret( const public_key& pub )const;

           signature         sign( const fcl::sha256& digest )const;
           compact_signature sign_compact( const fcl::sha256& digest )const;
           bool              verify( const fcl::sha256& digest, const signature& sig );

           public_key get_public_key()const;

           inline friend bool operator==( const private_key& a, const private_key& b )
           {
            return a.get_secret() == b.get_secret();
           }
           inline friend std::strong_ordering operator<=>( const private_key& a, const private_key& b ) {
              return a.get_secret() <=> b.get_secret();
           }

        private:
           fcl::fwd<detail::private_key_impl,32> my;
    };

     /**
       * Shims
       */
     struct public_key_shim : public crypto::shim<public_key_data> {
        using crypto::shim<public_key_data>::shim;

        bool valid()const {
           return public_key(_data).valid();
        }
     };

     struct signature_shim : public crypto::shim<compact_signature> {
        using public_key_type = public_key_shim;
        using crypto::shim<compact_signature>::shim;

        public_key_type recover(const sha256& digest, bool check_canonical) const {
           return public_key_type(public_key(_data, digest, check_canonical).serialize());
        }
     };

     struct private_key_shim : public crypto::shim<private_key_secret> {
        using crypto::shim<private_key_secret>::shim;
        using signature_type = signature_shim;
        using public_key_type = public_key_shim;

        signature_type sign( const sha256& digest, bool require_canonical = true ) const
        {
           return signature_type(private_key::regenerate(_data).sign_compact(digest));
        }

        public_key_type get_public_key( ) const
        {
           return public_key_type(private_key::regenerate(_data).get_public_key().serialize());
        }

        sha512 generate_shared_secret( const public_key_type &pub_key ) const
        {
           return private_key::regenerate(_data).get_shared_secret(public_key(pub_key.serialize()));
        }

        static private_key_shim generate()
        {
           return private_key_shim(private_key::generate().get_secret());
        }
     };

     compact_signature signature_from_ecdsa(const public_key_data& pub, fcl::ecdsa_sig& sig, const fcl::sha256& d);

  } // namespace r1
  } // namespace crypto

} // namespace fcl
#include <fcl/reflect/reflect.hpp>

FCL_REFLECT_TYPENAME( fcl::crypto::r1::private_key )
FCL_REFLECT_TYPENAME( fcl::crypto::r1::public_key )
FCL_REFLECT_DERIVED( fcl::crypto::r1::public_key_shim, (fcl::crypto::shim<fcl::crypto::r1::public_key_data>), BOOST_PP_SEQ_NIL )
FCL_REFLECT_DERIVED( fcl::crypto::r1::signature_shim, (fcl::crypto::shim<fcl::crypto::r1::compact_signature>), BOOST_PP_SEQ_NIL )
FCL_REFLECT_DERIVED( fcl::crypto::r1::private_key_shim, (fcl::crypto::shim<fcl::crypto::r1::private_key_secret>), BOOST_PP_SEQ_NIL )
