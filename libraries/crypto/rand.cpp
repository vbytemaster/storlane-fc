#include <openssl/rand.h>
#include <fcl/crypto/openssl.hpp>
#include <fcl/exception/exception.hpp>
#include <fcl/core/fwd_impl.hpp>


namespace fcl {

void rand_bytes(char* buf, int count)
{
  int result = RAND_bytes((unsigned char*)buf, count);
  if (result != 1)
    FCL_THROW("Error calling OpenSSL's RAND_bytes(): ${code}", ("code", (uint32_t)ERR_get_error()));
}

void rand_pseudo_bytes(char* buf, int count)
{
  rand_bytes(buf, count);
}

}  // namespace fcl
