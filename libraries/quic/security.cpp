module;

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <memory>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/x509.h>

module fcl.quic.security;

namespace fcl::quic {
namespace {

[[nodiscard]] bool is_hex(char value) noexcept {
   return std::isxdigit(static_cast<unsigned char>(value)) != 0;
}

struct bio_deleter {
   void operator()(BIO* bio) const noexcept {
      BIO_free(bio);
   }
};

struct x509_deleter {
   void operator()(X509* certificate) const noexcept {
      X509_free(certificate);
   }
};

[[nodiscard]] std::vector<std::uint8_t> der_from_pem_certificate(std::string_view certificate_pem) {
   auto bio = std::unique_ptr<BIO, bio_deleter>{BIO_new_mem_buf(certificate_pem.data(), static_cast<int>(certificate_pem.size()))};
   if (!bio) {
      throw_quic_error(error_kind::tls_failed, "failed to allocate certificate BIO");
   }
   auto certificate = std::unique_ptr<X509, x509_deleter>{PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr)};
   if (!certificate) {
      throw_quic_error(error_kind::tls_failed, "failed to parse certificate PEM");
   }
   const auto length = i2d_X509(certificate.get(), nullptr);
   if (length <= 0) {
      throw_quic_error(error_kind::tls_failed, "failed to DER-encode certificate");
   }
   auto der = std::vector<std::uint8_t>(static_cast<std::size_t>(length));
   auto* out = der.data();
   if (i2d_X509(certificate.get(), &out) != length) {
      throw_quic_error(error_kind::tls_failed, "failed to DER-encode certificate");
   }
   return der;
}

} // namespace

std::string normalize_sha256_fingerprint(std::string_view value) {
   auto normalized = std::string{};
   normalized.reserve(value.size());
   for (const auto ch : value) {
      if (ch == ':' || ch == '-' || std::isspace(static_cast<unsigned char>(ch)) != 0) {
         continue;
      }
      if (!is_hex(ch)) {
         throw_quic_error(error_kind::invalid_options, "invalid SHA-256 fingerprint");
      }
      normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
   }
   if (normalized.size() != 64) {
      throw_quic_error(error_kind::invalid_options, "SHA-256 fingerprint must contain 32 bytes");
   }
   return normalized;
}

std::string sha256_fingerprint(std::span<const std::uint8_t> data) {
   auto digest = std::array<unsigned char, SHA256_DIGEST_LENGTH>{};
   SHA256(data.data(), data.size(), digest.data());

   auto out = std::ostringstream{};
   out << std::hex << std::setfill('0');
   for (const auto byte : digest) {
      out << std::setw(2) << static_cast<unsigned>(byte);
   }
   return out.str();
}

std::string certificate_sha256_fingerprint_from_pem(std::string_view certificate_pem) {
   return sha256_fingerprint(der_from_pem_certificate(certificate_pem));
}

bool verify_peer_certificate(const peer_certificate& certificate, const security_options& options) {
   if (!options.verify_peer) {
      return true;
   }

   const auto actual = normalize_sha256_fingerprint(certificate.sha256_fingerprint.empty()
                                                       ? sha256_fingerprint(certificate.der)
                                                       : certificate.sha256_fingerprint);
   if (options.expected_sha256_fingerprint) {
      if (actual != normalize_sha256_fingerprint(*options.expected_sha256_fingerprint)) {
         return false;
      }
   }

   if (options.verifier && !options.verifier(certificate)) {
      return false;
   }

   return true;
}

} // namespace fcl::quic
