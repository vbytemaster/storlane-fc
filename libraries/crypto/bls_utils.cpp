module;
#include <bls12-381/bls12-381.hpp>
#include <cstdint>
#include <span>

module fcl.crypto.bls_utils;

import fcl.crypto.bls_public_key;
import fcl.crypto.bls_signature;

namespace fcl::crypto::blslib {

   bool verify(const bls_public_key& pubkey,
               std::span<const uint8_t> message,
               const bls_signature& signature) {
      return bls12_381::verify(pubkey.jacobian_montgomery_le(), message, signature.jacobian_montgomery_le());
   };

} // fcl::crypto::blslib
