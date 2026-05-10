add_library(
   fcl_log OBJECT
   appender.cpp
   console_appender.cpp
   log_message.cpp
   logger.cpp
   logger_config.cpp
)

target_include_directories(fcl_log PUBLIC ${FCL_PUBLIC_INCLUDE_DIRS})
target_link_libraries(fcl_log PUBLIC Boost::headers)
target_compile_definitions(
   fcl_log
   PUBLIC OPENSSL_API_COMPAT=0x30000000L
          OPENSSL_NO_DEPRECATED
)
