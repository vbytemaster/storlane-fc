module;

export module fcl.crypto.kdf;

import fcl.crypto.types;

export namespace fcl::crypto {

[[nodiscard]] bytes derive_hkdf_sha256(const hkdf_sha256_request& request);
[[nodiscard]] bytes derive_scrypt(const scrypt_request& request);

} // namespace fcl::crypto
