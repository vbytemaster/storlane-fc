module;
#include <cstdint>
#include <exception>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <string>

module fcl.crypto.rand;

import fcl.crypto.openssl;

namespace fcl {

void rand_bytes(char* buf, int count) {
   int result = RAND_bytes((unsigned char*)buf, count);
   if (result != 1)
      throw std::runtime_error("Error calling OpenSSL's RAND_bytes(): " +
                               std::to_string(static_cast<uint32_t>(ERR_get_error())));
}

void rand_pseudo_bytes(char* buf, int count) {
   rand_bytes(buf, count);
}

} // namespace fcl
