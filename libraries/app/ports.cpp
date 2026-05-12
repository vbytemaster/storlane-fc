module;

#include <string>

module fcl.app.ports;

namespace fcl::app {

port_error::port_error(std::string message) : std::runtime_error(std::move(message)) {}

port_registry::port_registry() = default;
port_registry::~port_registry() = default;

std::size_t port_registry::size() const noexcept {
   return ports_.size();
}

void port_registry::clear() noexcept {
   ports_.clear();
}

} // namespace fcl::app
