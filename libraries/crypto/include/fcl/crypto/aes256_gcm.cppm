module;

export module fcl.crypto.aes256_gcm;

import fcl.crypto.types;

export namespace fcl::crypto::aes256_gcm {

[[nodiscard]] fcl::crypto::aes256_gcm_ciphertext encrypt(
   const fcl::crypto::aes256_gcm_encrypt_request& request);

[[nodiscard]] fcl::crypto::bytes decrypt(
   const fcl::crypto::aes256_gcm_decrypt_request& request);

} // namespace fcl::crypto::aes256_gcm
