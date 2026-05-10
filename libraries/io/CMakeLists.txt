add_library(
   fcl_io OBJECT
   datastream.cpp
   fstream.cpp
   varint.cpp
)

target_include_directories(fcl_io PUBLIC ${FCL_PUBLIC_INCLUDE_DIRS})
target_link_libraries(fcl_io PUBLIC Boost::headers)
target_compile_definitions(
   fcl_io
   PUBLIC OPENSSL_API_COMPAT=0x30000000L
          OPENSSL_NO_DEPRECATED
)
