add_library(fcl_json OBJECT json.cpp)

target_include_directories(fcl_json PUBLIC ${FCL_PUBLIC_INCLUDE_DIRS})
target_link_libraries(fcl_json PUBLIC Boost::headers)
target_compile_definitions(
   fcl_json
   PUBLIC OPENSSL_API_COMPAT=0x30000000L
          OPENSSL_NO_DEPRECATED
)
