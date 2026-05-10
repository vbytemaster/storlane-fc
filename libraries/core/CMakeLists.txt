file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/generated")
configure_file(
   "${CMAKE_CURRENT_SOURCE_DIR}/git_revision.cpp.in"
   "${CMAKE_CURRENT_BINARY_DIR}/generated/git_revision.cpp"
   @ONLY
)

add_library(
   fcl_core OBJECT
   filesystem.cpp
   mock_time.cpp
   string.cpp
   time.cpp
   uint128.cpp
   utf8.cpp
   "${CMAKE_CURRENT_BINARY_DIR}/generated/git_revision.cpp"
)

target_include_directories(fcl_core PUBLIC ${FCL_PUBLIC_INCLUDE_DIRS})
target_link_libraries(fcl_core PUBLIC Boost::headers)
target_compile_definitions(
   fcl_core
   PUBLIC OPENSSL_API_COMPAT=0x30000000L
          OPENSSL_NO_DEPRECATED
)

if(FCL_ENABLE_MODULES AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
   target_sources(
      fcl_core
      PUBLIC
         FILE_SET CXX_MODULES
         BASE_DIRS include
         FILES include/fcl/core/version.cppm
   )
elseif(FCL_ENABLE_MODULES)
   message(STATUS "FCL module file sets requested, but AppleClang cannot provide CMake's module dependency scan; building textual headers only")
endif()
