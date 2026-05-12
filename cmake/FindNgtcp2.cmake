find_path(
   NGTCP2_INCLUDE_DIR
   NAMES ngtcp2/ngtcp2.h
   HINTS
      ${NGTCP2_ROOT}
      $ENV{NGTCP2_ROOT}
      /opt/homebrew/opt/libngtcp2
      /usr/local/opt/libngtcp2
)

find_library(
   NGTCP2_LIBRARY
   NAMES ngtcp2
   HINTS
      ${NGTCP2_ROOT}
      $ENV{NGTCP2_ROOT}
      /opt/homebrew/opt/libngtcp2
      /usr/local/opt/libngtcp2
   PATH_SUFFIXES lib
)

find_library(
   NGTCP2_CRYPTO_OSSL_LIBRARY
   NAMES ngtcp2_crypto_ossl
   HINTS
      ${NGTCP2_ROOT}
      $ENV{NGTCP2_ROOT}
      /opt/homebrew/opt/libngtcp2
      /usr/local/opt/libngtcp2
   PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
   Ngtcp2
   REQUIRED_VARS NGTCP2_INCLUDE_DIR NGTCP2_LIBRARY NGTCP2_CRYPTO_OSSL_LIBRARY
)

if(Ngtcp2_FOUND AND NOT TARGET Ngtcp2::ngtcp2)
   add_library(Ngtcp2::ngtcp2 UNKNOWN IMPORTED)
   set_target_properties(
      Ngtcp2::ngtcp2
      PROPERTIES
         IMPORTED_LOCATION "${NGTCP2_LIBRARY}"
         INTERFACE_INCLUDE_DIRECTORIES "${NGTCP2_INCLUDE_DIR}"
   )
endif()

if(Ngtcp2_FOUND AND NOT TARGET Ngtcp2::crypto_ossl)
   add_library(Ngtcp2::crypto_ossl UNKNOWN IMPORTED)
   set_target_properties(
      Ngtcp2::crypto_ossl
      PROPERTIES
         IMPORTED_LOCATION "${NGTCP2_CRYPTO_OSSL_LIBRARY}"
         INTERFACE_INCLUDE_DIRECTORIES "${NGTCP2_INCLUDE_DIR}"
   )
endif()
