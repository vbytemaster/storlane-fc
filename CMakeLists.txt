cmake_minimum_required(VERSION 3.31)

project(storlane-fc VERSION 0.1.0 LANGUAGES C CXX ASM)

include(GNUInstallDirs)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
   include(CTest)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FCL_ENABLE_MODULES "Build FCL public module interfaces when supported" ON)
option(FCL_ENABLE_VENDOR_TESTS "Build donor dependency tests and benchmarks" OFF)

if("${CMAKE_GENERATOR}" STREQUAL "Ninja")
   add_compile_options(-fdiagnostics-color=always)
endif()

if(APPLE)
   message(STATUS "Configuring storlane-fc on macOS")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else()
   message(STATUS "Configuring storlane-fc on Linux")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -latomic")
endif()

add_compile_definitions(BOOST_TEST_DEFAULTS_TO_CORE_DUMP)
add_compile_definitions(BOOST_UNORDERED_DISABLE_NEON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

find_package(
   Boost 1.90 REQUIRED CONFIG
   COMPONENTS headers date_time chrono iostreams thread
)

find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto)

function(fcl_add_header_only_boost_target component)
   if(NOT TARGET Boost::${component})
      add_library(Boost::${component} INTERFACE IMPORTED)
      set_property(
         TARGET Boost::${component}
         APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::headers
      )
   endif()
endfunction()

fcl_add_header_only_boost_target(asio)
fcl_add_header_only_boost_target(beast)
fcl_add_header_only_boost_target(crc)
fcl_add_header_only_boost_target(dll)
fcl_add_header_only_boost_target(interprocess)
fcl_add_header_only_boost_target(multi_index)
fcl_add_header_only_boost_target(multiprecision)

if(NOT TARGET Boost::included_unit_test_framework)
   add_library(Boost::included_unit_test_framework INTERFACE IMPORTED)
   set_property(
      TARGET Boost::included_unit_test_framework
      APPEND PROPERTY INTERFACE_LINK_LIBRARIES Boost::headers
   )
endif()

find_package(Git QUIET)
set(FCL_GIT_REVISION_SHA "unknown")
set(FCL_GIT_REVISION_UNIX_TIMESTAMP 0)

if(Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
   execute_process(
      COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE FCL_GIT_REVISION_SHA
      RESULT_VARIABLE _fcl_git_rev_parse_result
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
   )
   if(NOT _fcl_git_rev_parse_result EQUAL 0)
      set(FCL_GIT_REVISION_SHA "unknown")
   endif()

   execute_process(
      COMMAND "${GIT_EXECUTABLE}" log -1 --format=%ct
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
      OUTPUT_VARIABLE FCL_GIT_REVISION_UNIX_TIMESTAMP
      RESULT_VARIABLE _fcl_git_log_result
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_QUIET
   )
   if(NOT _fcl_git_log_result EQUAL 0)
      set(FCL_GIT_REVISION_UNIX_TIMESTAMP 0)
   endif()
endif()

find_path(GMP_INCLUDE_DIR NAMES gmp.h)
find_library(GMP_LIBRARY gmp)
if(NOT GMP_LIBRARY MATCHES ${CMAKE_SHARED_LIBRARY_SUFFIX})
   message(FATAL_ERROR "GMP shared library not found")
endif()
message(STATUS "GMP: ${GMP_LIBRARY}, ${GMP_INCLUDE_DIR}")
add_library(GMP::gmp SHARED IMPORTED)
set_target_properties(
   GMP::gmp PROPERTIES
   IMPORTED_LOCATION ${GMP_LIBRARY}
   INTERFACE_INCLUDE_DIRECTORIES ${GMP_INCLUDE_DIR}
)

set(
   FCL_PUBLIC_INCLUDE_DIRS
   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/core/include"
   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/exception/include"
   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/reflect/include"
   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/variant/include"
   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/io/include"
   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/raw/include"
   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/json/include"
   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/log/include"
   "${CMAKE_CURRENT_SOURCE_DIR}/libraries/crypto/include"
)

add_library(secp256k1-internal INTERFACE)
target_include_directories(secp256k1-internal INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/secp256k1/src")
target_compile_definitions(
   secp256k1-internal INTERFACE
      ENABLE_MODULE_RECOVERY=1
      COMB_BLOCKS=43
      COMB_TEETH=6
      ECMULT_WINDOW_SIZE=15
      SECP256K1_STATIC=1
)
if(CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
   target_compile_definitions(secp256k1-internal INTERFACE USE_ASM_X86_64=1)
endif()
target_compile_options(secp256k1-internal INTERFACE -Wno-unused-function)

add_library(
   secp256k1 STATIC
   vendor/secp256k1/src/secp256k1.c
   vendor/secp256k1/src/precomputed_ecmult.c
   vendor/secp256k1/src/precomputed_ecmult_gen.c
)
target_include_directories(
   secp256k1 PUBLIC
      "${CMAKE_CURRENT_SOURCE_DIR}/vendor/secp256k1"
      "${CMAKE_CURRENT_SOURCE_DIR}/vendor/secp256k1/include"
)
target_link_libraries(secp256k1 PRIVATE secp256k1-internal)

if(FCL_ENABLE_VENDOR_TESTS)
   add_executable(secp256k1-bench vendor/secp256k1/src/bench.c)
   target_link_libraries(secp256k1-bench secp256k1 secp256k1-internal)

   add_executable(secp256k1-tests vendor/secp256k1/src/tests.c)
   target_link_libraries(secp256k1-tests secp256k1 secp256k1-internal)
   add_test(NAME secp256k1-tests COMMAND secp256k1-tests)

   add_executable(secp256k1-exhaustive-tests vendor/secp256k1/src/tests_exhaustive.c)
   target_link_libraries(secp256k1-exhaustive-tests secp256k1 secp256k1-internal)
   add_test(NAME secp256k1-exhaustive-tests COMMAND secp256k1-exhaustive-tests)
endif()

add_subdirectory(vendor/bn256/src)
add_subdirectory(vendor/bls12-381 EXCLUDE_FROM_ALL)

if(NOT FCL_ENABLE_VENDOR_TESTS)
   file(
      WRITE "${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake"
      "set(CTEST_CUSTOM_TESTS_IGNORE bls12-381 bls12-381-nobmi2)\n"
   )
endif()

add_subdirectory(libraries/core)
add_subdirectory(libraries/exception)
add_subdirectory(libraries/reflect)
add_subdirectory(libraries/variant)
add_subdirectory(libraries/io)
add_subdirectory(libraries/raw)
add_subdirectory(libraries/json)
add_subdirectory(libraries/log)
add_subdirectory(libraries/crypto)

add_library(
   fcl STATIC
   $<TARGET_OBJECTS:fcl_core>
   $<TARGET_OBJECTS:fcl_exception>
   $<TARGET_OBJECTS:fcl_variant>
   $<TARGET_OBJECTS:fcl_io>
   $<TARGET_OBJECTS:fcl_json>
   $<TARGET_OBJECTS:fcl_log>
   $<TARGET_OBJECTS:fcl_crypto>
)

target_include_directories(fcl PUBLIC ${FCL_PUBLIC_INCLUDE_DIRS})

target_compile_definitions(
   fcl
   PUBLIC OPENSSL_API_COMPAT=0x30000000L
          OPENSSL_NO_DEPRECATED
)

target_link_libraries(
   fcl PUBLIC
      fcl_core
      fcl_exception
      fcl_reflect
      fcl_variant
      fcl_io
      fcl_raw
      fcl_json
      fcl_log
      fcl_crypto
      GMP::gmp
      Boost::date_time
      Boost::chrono
      Boost::iostreams
      Boost::interprocess
      Boost::multi_index
      Boost::dll
      Boost::multiprecision
      Boost::thread
      Boost::crc
      Threads::Threads
      OpenSSL::Crypto
      ${PLATFORM_SPECIFIC_LIBS}
      ${CMAKE_DL_LIBS}
      secp256k1
      bls12-381
)

if(WIN32)
   target_link_libraries(fcl PUBLIC ws2_32 mswsock userenv)
endif()

if(APPLE)
   find_library(security_framework Security)
   find_library(corefoundation_framework CoreFoundation)
   target_link_libraries(fcl PUBLIC ${security_framework} ${corefoundation_framework})
endif()

if(DEFINED BUILD_TESTING AND BUILD_TESTING)
   add_subdirectory(tests)
endif()

install(
   TARGETS fcl
   LIBRARY DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR} COMPONENT dev EXCLUDE_FROM_ALL
   ARCHIVE DESTINATION ${CMAKE_INSTALL_FULL_LIBDIR} COMPONENT dev EXCLUDE_FROM_ALL
)

install(
   DIRECTORY libraries/core/include/fcl
             libraries/exception/include/fcl
             libraries/reflect/include/fcl
             libraries/variant/include/fcl
             libraries/io/include/fcl
             libraries/raw/include/fcl
             libraries/json/include/fcl
             libraries/log/include/fcl
             libraries/crypto/include/fcl
   DESTINATION ${CMAKE_INSTALL_FULL_INCLUDEDIR} COMPONENT dev EXCLUDE_FROM_ALL
)
