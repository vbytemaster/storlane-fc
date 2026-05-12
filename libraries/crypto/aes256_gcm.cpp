module;

#include <openssl/evp.h>

#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <string>

module fcl.crypto.aes256_gcm;

namespace fcl::crypto::aes256_gcm {
namespace {

void require_nonce(const bytes& nonce)
{
   if (nonce.size() != aes_gcm_nonce_size) {
      throw error{error_kind::invalid_nonce, "AES-256-GCM requires 12-byte nonce"};
   }
}

void require_tag(const bytes& tag)
{
   if (tag.size() != aes_gcm_tag_size) {
      throw error{error_kind::invalid_tag, "AES-256-GCM requires 16-byte tag"};
   }
}

int checked_update_size(std::size_t size, const char* label)
{
   if (size > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
      throw error{error_kind::invalid_options, std::string(label) + " is too large"};
   }
   return static_cast<int>(size);
}

using ctx_ptr = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

[[nodiscard]] ctx_ptr make_context()
{
   auto context = ctx_ptr{EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free};
   if (!context) {
      throw error{error_kind::backend_error, "failed to allocate AES-GCM context"};
   }
   return context;
}

} // namespace

aes256_gcm_ciphertext encrypt(const aes256_gcm_encrypt_request& request)
{
   require_nonce(request.nonce);

   auto context = make_context();
   auto out = bytes(request.plaintext.size());
   auto out_size = int{};
   auto total_size = int{};
   auto tag = bytes(aes_gcm_tag_size);
   auto final_out = std::array<std::uint8_t, aes_gcm_tag_size>{};

   if (EVP_EncryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1 ||
       EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(request.nonce.size()), nullptr) != 1 ||
       EVP_EncryptInit_ex(context.get(), nullptr, nullptr, request.key.bytes.data(), request.nonce.data()) != 1) {
      throw error{error_kind::backend_error, "failed to initialize AES-GCM encryption"};
   }
   if (!request.aad.empty() &&
       EVP_EncryptUpdate(
          context.get(),
          nullptr,
          &out_size,
          request.aad.data(),
          checked_update_size(request.aad.size(), "AES-GCM AAD")) != 1) {
      throw error{error_kind::backend_error, "failed to apply AES-GCM AAD"};
   }
   out_size = 0;
   if (!request.plaintext.empty() &&
       EVP_EncryptUpdate(
          context.get(),
          out.data(),
          &out_size,
          request.plaintext.data(),
          checked_update_size(request.plaintext.size(), "AES-GCM plaintext")) != 1) {
      throw error{error_kind::backend_error, "failed to encrypt AES-GCM payload"};
   }
   total_size = out_size;
   if (EVP_EncryptFinal_ex(context.get(), final_out.data(), &out_size) != 1) {
      throw error{error_kind::backend_error, "failed to finalize AES-GCM encryption"};
   }
   total_size += out_size;
   out.resize(static_cast<std::size_t>(total_size));
   if (EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_GET_TAG, static_cast<int>(tag.size()), tag.data()) != 1) {
      throw error{error_kind::backend_error, "failed to read AES-GCM tag"};
   }

   return aes256_gcm_ciphertext{
      .nonce = request.nonce,
      .tag = std::move(tag),
      .ciphertext = std::move(out),
   };
}

bytes decrypt(const aes256_gcm_decrypt_request& request)
{
   require_nonce(request.encrypted.nonce);
   require_tag(request.encrypted.tag);

   auto context = make_context();
   auto out = bytes(request.encrypted.ciphertext.size());
   auto out_size = int{};
   auto total_size = int{};
   auto final_out = std::array<std::uint8_t, aes_gcm_tag_size>{};

   if (EVP_DecryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1 ||
       EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_IVLEN, static_cast<int>(request.encrypted.nonce.size()), nullptr) != 1 ||
       EVP_DecryptInit_ex(context.get(), nullptr, nullptr, request.key.bytes.data(), request.encrypted.nonce.data()) != 1) {
      throw error{error_kind::backend_error, "failed to initialize AES-GCM decryption"};
   }
   if (!request.aad.empty() &&
       EVP_DecryptUpdate(
          context.get(),
          nullptr,
          &out_size,
          request.aad.data(),
          checked_update_size(request.aad.size(), "AES-GCM AAD")) != 1) {
      throw error{error_kind::backend_error, "failed to apply AES-GCM AAD"};
   }
   out_size = 0;
   if (!request.encrypted.ciphertext.empty() &&
       EVP_DecryptUpdate(
          context.get(),
          out.data(),
          &out_size,
          request.encrypted.ciphertext.data(),
          checked_update_size(request.encrypted.ciphertext.size(), "AES-GCM ciphertext")) != 1) {
      throw error{error_kind::backend_error, "failed to decrypt AES-GCM payload"};
   }
   total_size = out_size;
   if (EVP_CIPHER_CTX_ctrl(
          context.get(),
          EVP_CTRL_GCM_SET_TAG,
          static_cast<int>(request.encrypted.tag.size()),
          const_cast<std::uint8_t*>(request.encrypted.tag.data())) != 1 ||
       EVP_DecryptFinal_ex(context.get(), final_out.data(), &out_size) != 1) {
      throw error{error_kind::authentication_failed, "AES-GCM authentication failed"};
   }
   total_size += out_size;
   out.resize(static_cast<std::size_t>(total_size));
   return out;
}

} // namespace fcl::crypto::aes256_gcm
