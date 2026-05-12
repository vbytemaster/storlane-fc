module;
#include <cstdint>
#include <span>

export module fcl.crypto.bls_utils;

import fcl.crypto.bls_private_key;
import fcl.crypto.bls_public_key;
import fcl.crypto.bls_signature;


export namespace fcl::crypto::blslib {

   bool verify(const bls_public_key& pubkey,
               std::span<const uint8_t> message,
               const bls_signature& signature);

} // fcl::crypto::blslib

