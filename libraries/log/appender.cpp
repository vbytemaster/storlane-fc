import fcl.log.appender;
import fcl.log.console_appender;
import fcl.log.logger_config;

namespace fcl {

static bool reg_console_appender = log_config::register_appender<console_appender>("console");

} // namespace fcl
