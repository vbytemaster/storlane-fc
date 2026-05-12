add_library(
   fcl_log OBJECT
   appender.cpp
   console_appender.cpp
   log_message.cpp
   logger.cpp
   logger_config.cpp
)
fcl_target_modules(fcl_log log)

target_include_directories(fcl_log PUBLIC ${FCL_PUBLIC_INCLUDE_DIRS})
target_link_libraries(
   fcl_log PUBLIC
      fcl_core
      fcl_reflect
      fcl_variant
      Boost::headers
   PRIVATE
      Boost::dll
)
target_compile_definitions(
   fcl_log
   PUBLIC OPENSSL_API_COMPAT=0x30000000L
          OPENSSL_NO_DEPRECATED
)
