module;

#include <memory>
#include <stdexcept>
#include <string>
#include <cstddef>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <utility>

export module fcl.app.ports;

export namespace fcl::app {

class port_error : public std::runtime_error {
public:
   explicit port_error(std::string message);
};

class port_base {
public:
   virtual ~port_base() = default;
};

template <typename Port>
class port_holder final : public port_base {
public:
   explicit port_holder(std::shared_ptr<Port> value)
      : value_(std::move(value)) {}

   std::shared_ptr<Port> value_;
};

class port_registry {
public:
   port_registry();
   ~port_registry();

   port_registry(const port_registry&) = delete;
   port_registry& operator=(const port_registry&) = delete;

   template <typename Port>
   void install(std::shared_ptr<Port> port) {
      static_assert(std::is_class_v<Port>, "app port must be a class/interface type");
      static_assert(std::has_virtual_destructor_v<Port>, "app port must have a virtual destructor");
      if (!port) {
         throw port_error{"cannot install null app port"};
      }
      const auto key = std::type_index(typeid(Port));
      if (ports_.contains(key)) {
         throw port_error{"duplicate app port installation"};
      }
      ports_.emplace(key, std::make_unique<port_holder<Port>>(std::move(port)));
   }

   template <typename Port>
   [[nodiscard]] std::shared_ptr<Port> try_get() const {
      const auto iterator = ports_.find(std::type_index(typeid(Port)));
      if (iterator == ports_.end()) {
         return {};
      }
      const auto* holder = dynamic_cast<const port_holder<Port>*>(iterator->second.get());
      if (holder == nullptr) {
         throw port_error{"app port type mismatch"};
      }
      return holder->value_;
   }

   template <typename Port>
   [[nodiscard]] std::shared_ptr<Port> get() const {
      auto port = try_get<Port>();
      if (!port) {
         throw port_error{"required app port is not installed"};
      }
      return port;
   }

   template <typename Port>
   void remove() {
      ports_.erase(std::type_index(typeid(Port)));
   }

   [[nodiscard]] std::size_t size() const noexcept;
   void clear() noexcept;

private:
   std::unordered_map<std::type_index, std::unique_ptr<port_base>> ports_;
};

} // namespace fcl::app
