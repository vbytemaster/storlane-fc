module;

#include <map>
#include <optional>
#include <string>

export module fcl.app.plugin_context;

import fcl.app.diagnostics;
import fcl.app.events;
import fcl.app.ports;
import fcl.app.signals;
import fcl.asio.task_scheduler;

export namespace fcl::app {

using config_view = std::map<std::string, std::string>;

class plugin_context {
public:
   plugin_context(
      fcl::asio::task_scheduler& scheduler,
      port_registry& ports,
      signal_bus& signals,
      event_bus& events,
      diagnostics_store* diagnostics = nullptr,
      config_view config = {});

   [[nodiscard]] fcl::asio::task_scheduler& scheduler() noexcept;
   [[nodiscard]] port_registry& ports() noexcept;
   [[nodiscard]] signal_bus& signals() noexcept;
   [[nodiscard]] event_bus& events() noexcept;
   [[nodiscard]] diagnostics_store* diagnostics() noexcept;
   [[nodiscard]] const config_view& config() const noexcept;
   [[nodiscard]] std::optional<std::string> config_value(const std::string& key) const;

private:
   fcl::asio::task_scheduler* scheduler_ = nullptr;
   port_registry* ports_ = nullptr;
   signal_bus* signals_ = nullptr;
   event_bus* events_ = nullptr;
   diagnostics_store* diagnostics_ = nullptr;
   config_view config_;
};

} // namespace fcl::app
