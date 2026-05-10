add_executable(
   test_fcl
   unit/main.cpp
   ../libraries/core/tests/test_escape_str.cpp
   ../libraries/core/tests/test_scoped_exit.cpp
   ../libraries/variant/tests/test_static_variant.cpp
   ../libraries/variant/tests/test_variant.cpp
   ../libraries/variant/tests/test_variant_dynamic_bitset.cpp
   ../libraries/variant/tests/test_variant_estimated_size.cpp
   ../libraries/raw/tests/test_raw.cpp
   ../libraries/json/tests/test_json.cpp
   ../libraries/crypto/tests/test_base64.cpp
   ../libraries/crypto/tests/test_blake2.cpp
   ../libraries/crypto/tests/test_bls.cpp
   ../libraries/crypto/tests/test_bls_legacy.cpp
   ../libraries/crypto/tests/test_cypher_suites.cpp
   ../libraries/crypto/tests/test_hash_functions.cpp
   ../libraries/crypto/tests/test_k1_recover.cpp
   ../libraries/crypto/tests/test_modular_arithmetic.cpp
   ../libraries/crypto/tests/test_webauthn.cpp
)

target_link_libraries(test_fcl fcl Boost::included_unit_test_framework)
add_test(NAME test_fcl COMMAND test_fcl WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
