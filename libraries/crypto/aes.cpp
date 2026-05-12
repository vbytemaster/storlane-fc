module;
#include <fcl/exception/macros.hpp>

#if defined(_WIN32)
# include <windows.h>
#endif

#include <openssl/err.h>
#include <openssl/evp.h>

#include <thread>
#include <mutex>
#include <fstream>
#include <functional>
#include <memory>

module fcl.crypto.aes;

import fcl.crypto.openssl;
import fcl.crypto.sha256;
import fcl.core.uint128;
import fcl.exception.exception;
import fcl.raw.raw;

namespace fcl {

struct aes_encoder::impl
{
   evp_cipher_ctx ctx;
};

aes_encoder::aes_encoder()
: my( std::make_unique<impl>() )
{}

aes_encoder::~aes_encoder() = default;


void aes_encoder::init( const fcl::sha256& key, const fcl::uint128& init_value )
{
    my->ctx.obj = EVP_CIPHER_CTX_new();
    /* Create and initialise the context */
    if(!my->ctx)
    {
        FCL_THROW("error allocating evp cipher context",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    /* Initialise the encryption operation. IMPORTANT - ensure you use a key
    *    and IV size appropriate for your cipher
    *    In this example we are using 256 bit AES (i.e. a 256 bit key). The
    *    IV size for *most* modes is the same as the block size. For AES this
    *    is 128 bits */
    if(1 != EVP_EncryptInit_ex(my->ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)&key, (unsigned char*)&init_value))
    {
        FCL_THROW("error during aes 256 cbc encryption init",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }
    EVP_CIPHER_CTX_set_padding( my->ctx, 0 );
}

uint32_t aes_encoder::encode( const char* plaintxt, uint32_t plaintext_len, char* ciphertxt )
{
    int ciphertext_len = 0;
    /* Provide the message to be encrypted, and obtain the encrypted output.
    *    * EVP_EncryptUpdate can be called multiple times if necessary
    *       */
    if(1 != EVP_EncryptUpdate(my->ctx, (unsigned char*)ciphertxt, &ciphertext_len, (const unsigned char*)plaintxt, plaintext_len))
    {
        FCL_THROW("error during aes 256 cbc encryption update",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }
    FCL_ASSERT(ciphertext_len == static_cast<int>(plaintext_len),
               "unexpected AES ciphertext length",
               fcl::error::ctx("ciphertext_len", ciphertext_len),
               fcl::error::ctx("plaintext_len", plaintext_len));
    return ciphertext_len;
}
#if 0
uint32_t aes_encoder::final_encode( char* ciphertxt )
{
    int ciphertext_len = 0;
    /* Finalise the encryption. Further ciphertext bytes may be written at
    *    * this stage.
    *       */
    if(1 != EVP_EncryptFinal_ex(my->ctx, (unsigned char*)ciphertxt, &ciphertext_len))
    {
        FCL_THROW("error during aes 256 cbc encryption final", fcl::error::ctx("s", ERR_error_string( ERR_get_error(), nullptr) ));
    }
    return ciphertext_len;
}
#endif


struct aes_decoder::impl
{
   evp_cipher_ctx ctx;
};

aes_decoder::aes_decoder()
: my( std::make_unique<impl>() )
{}

aes_decoder::~aes_decoder() = default;

void aes_decoder::init( const fcl::sha256& key, const fcl::uint128& init_value )
{
    my->ctx.obj = EVP_CIPHER_CTX_new();
    /* Create and initialise the context */
    if(!my->ctx)
    {
        FCL_THROW("error allocating evp cipher context",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    /* Initialise the encryption operation. IMPORTANT - ensure you use a key
    *    and IV size appropriate for your cipher
    *    In this example we are using 256 bit AES (i.e. a 256 bit key). The
    *    IV size for *most* modes is the same as the block size. For AES this
    *    is 128 bits */
    if(1 != EVP_DecryptInit_ex(my->ctx, EVP_aes_256_cbc(), NULL, (unsigned char*)&key, (unsigned char*)&init_value))
    {
        FCL_THROW("error during aes 256 cbc encryption init",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }
    EVP_CIPHER_CTX_set_padding( my->ctx, 0 );
}

uint32_t aes_decoder::decode( const char* ciphertxt, uint32_t ciphertxt_len, char* plaintext )
{
    int plaintext_len = 0;
    /* Provide the message to be decrypted, and obtain the decrypted output.
    *    * EVP_DecryptUpdate can be called multiple times if necessary
    *       */
	if (1 != EVP_DecryptUpdate(my->ctx, (unsigned char*)plaintext, &plaintext_len, (const unsigned char*)ciphertxt, ciphertxt_len))
    {
        FCL_THROW("error during aes 256 cbc decryption update",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }
    FCL_ASSERT(ciphertxt_len == static_cast<unsigned>(plaintext_len),
               "unexpected AES plaintext length",
               fcl::error::ctx("ciphertxt_len", ciphertxt_len),
               fcl::error::ctx("plaintext_len", plaintext_len));
	return plaintext_len;
}
#if 0
uint32_t aes_decoder::final_decode( char* plaintext )
{
    return 0;
    int ciphertext_len = 0;
    /* Finalise the encryption. Further ciphertext bytes may be written at
    *    * this stage.
    *       */
    if(1 != EVP_DecryptFinal_ex(my->ctx, (unsigned char*)plaintext, &ciphertext_len))
    {
        FCL_THROW("error during aes 256 cbc encryption final", fcl::error::ctx("s", ERR_error_string( ERR_get_error(), nullptr) ));
    }
    return ciphertext_len;
}
#endif












/** example method from wiki.opensslfoundation.com */
unsigned aes_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key,
                     unsigned char *iv, unsigned char *ciphertext)
{
    evp_cipher_ctx ctx( EVP_CIPHER_CTX_new() );

    int len = 0;
    unsigned ciphertext_len = 0;

    /* Create and initialise the context */
    if(!ctx)
    {
        FCL_THROW("error allocating evp cipher context",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    /* Initialise the encryption operation. IMPORTANT - ensure you use a key
    *    and IV size appropriate for your cipher
    *    In this example we are using 256 bit AES (i.e. a 256 bit key). The
    *    IV size for *most* modes is the same as the block size. For AES this
    *    is 128 bits */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        FCL_THROW("error during aes 256 cbc encryption init",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    /* Provide the message to be encrypted, and obtain the encrypted output.
    *    * EVP_EncryptUpdate can be called multiple times if necessary
    *       */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
    {
        FCL_THROW("error during aes 256 cbc encryption update",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }
    ciphertext_len = len;

    /* Finalise the encryption. Further ciphertext bytes may be written at
    *    * this stage.
    *       */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
    {
        FCL_THROW("error during aes 256 cbc encryption final",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }
    ciphertext_len += len;

    return ciphertext_len;
}

unsigned aes_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
                     unsigned char *iv, unsigned char *plaintext)
{
    evp_cipher_ctx ctx( EVP_CIPHER_CTX_new() );
    int len = 0;
    unsigned plaintext_len = 0;

    /* Create and initialise the context */
    if(!ctx)
    {
        FCL_THROW("error allocating evp cipher context",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    /* Initialise the decryption operation. IMPORTANT - ensure you use a key
    *    * and IV size appropriate for your cipher
    *       * In this example we are using 256 bit AES (i.e. a 256 bit key). The
    *          * IV size for *most* modes is the same as the block size. For AES this
    *             * is 128 bits */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
    {
        FCL_THROW("error during aes 256 cbc decrypt init",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    /* Provide the message to be decrypted, and obtain the plaintext output.
    *    * EVP_DecryptUpdate can be called multiple times if necessary
    *       */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    {
        FCL_THROW("error during aes 256 cbc decrypt update",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    plaintext_len = len;

    /* Finalise the decryption. Further plaintext bytes may be written at
    *    * this stage.
    *       */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    {
        FCL_THROW("error during aes 256 cbc decrypt final",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }
    plaintext_len += len;

    return plaintext_len;
}

unsigned aes_cfb_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key,
                         unsigned char *iv, unsigned char *plaintext)
{
    evp_cipher_ctx ctx( EVP_CIPHER_CTX_new() );
    int len = 0;
    unsigned plaintext_len = 0;

    /* Create and initialise the context */
    if(!ctx)
    {
        FCL_THROW("error allocating evp cipher context",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    /* Initialise the decryption operation. IMPORTANT - ensure you use a key
    *    * and IV size appropriate for your cipher
    *       * In this example we are using 256 bit AES (i.e. a 256 bit key). The
    *          * IV size for *most* modes is the same as the block size. For AES this
    *             * is 128 bits */
    if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cfb128(), NULL, key, iv))
    {
        FCL_THROW("error during aes 256 cbc decrypt init",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    /* Provide the message to be decrypted, and obtain the plaintext output.
    *    * EVP_DecryptUpdate can be called multiple times if necessary
    *       */
    if(1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
    {
        FCL_THROW("error during aes 256 cbc decrypt update",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }

    plaintext_len = len;

    /* Finalise the decryption. Further plaintext bytes may be written at
    *    * this stage.
    *       */
    if(1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len))
    {
        FCL_THROW("error during aes 256 cbc decrypt final",
                  fcl::error::ctx("s", ERR_error_string(ERR_get_error(), nullptr)));
    }
    plaintext_len += len;

    return plaintext_len;
}

std::vector<char> aes_encrypt( const fcl::sha512& key, const std::vector<char>& plain_text  )
{
    std::vector<char> cipher_text(plain_text.size()+16);
    auto cipher_len = aes_encrypt( (unsigned char*)plain_text.data(), (int)plain_text.size(),
                                   (unsigned char*)&key, ((unsigned char*)&key)+32,
                                   (unsigned char*)cipher_text.data() );
    FCL_ASSERT( cipher_len <= cipher_text.size() );
    cipher_text.resize(cipher_len);
    return cipher_text;

}
std::vector<char> aes_decrypt( const fcl::sha512& key, const std::vector<char>& cipher_text )
{
    std::vector<char> plain_text( cipher_text.size() );
    auto plain_len = aes_decrypt( (unsigned char*)cipher_text.data(), (int)cipher_text.size(),
                                 (unsigned char*)&key, ((unsigned char*)&key)+32,
                                 (unsigned char*)plain_text.data() );
    plain_text.resize(plain_len);
    return plain_text;
}


/** encrypts plain_text and then includes a checksum that enables us to verify the integrety of
 * the file / key prior to decryption.
 */
void              aes_save( const std::filesystem::path& file, const fcl::sha512& key, std::vector<char> plain_text )
{ try {
   auto cipher = aes_encrypt( key, plain_text );
   fcl::sha512::encoder check_enc;
   fcl::raw::pack( check_enc, key );
   fcl::raw::pack( check_enc, cipher );
   auto check = check_enc.result();

   std::ofstream out(file.generic_string().c_str());
   fcl::raw::pack( out, check );
   fcl::raw::pack( out, cipher );
} FCL_CAPTURE_AND_RETHROW("AES file operation failed", fcl::error::ctx("file", file.generic_string())) }

/**
 *  recovers the plain_text saved via aes_save()
 */
std::vector<char> aes_load( const std::filesystem::path& file, const fcl::sha512& key )
{ try {
   FCL_ASSERT( std::filesystem::exists( file ) );

   std::ifstream in( file.generic_string().c_str(), std::ifstream::binary );
   fcl::sha512 check;
   std::vector<char> cipher;

   fcl::raw::unpack( in, check );
   fcl::raw::unpack( in, cipher );

   fcl::sha512::encoder check_enc;
   fcl::raw::pack( check_enc, key );
   fcl::raw::pack( check_enc, cipher );

   FCL_ASSERT( check_enc.result() == check );

   return aes_decrypt( key, cipher );
} FCL_CAPTURE_AND_RETHROW("AES file operation failed", fcl::error::ctx("file", file.generic_string())) }

}  // namespace fcl
