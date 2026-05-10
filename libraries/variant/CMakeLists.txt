add_library(
   fcl_variant OBJECT
   variant.cpp
   variant_object.cpp
)

target_include_directories(fcl_variant PUBLIC ${FCL_PUBLIC_INCLUDE_DIRS})
target_link_libraries(fcl_variant PUBLIC Boost::headers)
target_compile_definitions(
   fcl_variant
   PUBLIC OPENSSL_API_COMPAT=0x30000000L
          OPENSSL_NO_DEPRECATED
)
