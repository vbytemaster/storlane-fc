add_library(fcl_exception OBJECT exception.cpp)

target_include_directories(fcl_exception PUBLIC ${FCL_PUBLIC_INCLUDE_DIRS})
target_link_libraries(fcl_exception PUBLIC Boost::headers)
target_compile_definitions(
   fcl_exception
   PUBLIC OPENSSL_API_COMPAT=0x30000000L
          OPENSSL_NO_DEPRECATED
)
