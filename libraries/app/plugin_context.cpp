module;

#include <optional>
#include <string>
#include <utility>

module fcl.app.plugin_context;

namespace fcl::app {

plugin_context::plugin_context(
   fcl::asio::task_scheduler& scheduler,
   port_registry& ports,
   signal_bus& signals,
   event_bus& events,
   diagnostics_store* diagnostics,
   config_view config)
   : scheduler_{&scheduler}
   , ports_{&ports}
   , signals_{&signals}
   , events_{&events}
   , diagnostics_{diagnostics}
   , config_{std::move(config)} {}

fcl::asio::task_scheduler& plugin_context::scheduler() noexcept {
   return *scheduler_;
}

port_registry& plugin_context::ports() noexcept {
   return *ports_;
}

signal_bus& plugin_context::signals() noexcept {
   return *signals_;
}

event_bus& plugin_context::events() noexcept {
   return *events_;
}

diagnostics_store* plugin_context::diagnostics() noexcept {
   return diagnostics_;
}

const config_view& plugin_context::config() const noexcept {
   return config_;
}

std::optional<std::string> plugin_context::config_value(const std::string& key) const {
   const auto iterator = config_.find(key);
   if (iterator == config_.end()) {
      return std::nullopt;
   }
   return iterator->second;
}

} // namespace fcl::app
