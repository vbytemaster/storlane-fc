#pragma once
#include <fcl/crypto/bls_private_key.hpp>
#include <fcl/crypto/bls_public_key.hpp>
#include <fcl/crypto/bls_signature.hpp>

namespace fcl::crypto::blslib {

   bool verify(const bls_public_key& pubkey,
               std::span<const uint8_t> message,
               const bls_signature& signature);

} // fcl::crypto::blslib
