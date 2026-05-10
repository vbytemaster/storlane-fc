#include <fcl/log/appender.hpp>
#include <fcl/log/console_appender.hpp>
#include <fcl/log/logger_config.hpp>


namespace fcl {

   static bool reg_console_appender = log_config::register_appender<console_appender>( "console" );


} // namespace fcl
