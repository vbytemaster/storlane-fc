module;

#include <boost/describe.hpp>
#include <boost/mp11/algorithm.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

export module fcl.schema.enums;

import fcl.schema.diagnostic;
import fcl.schema.value_kind;

export namespace fcl::schema {

[[nodiscard]] inline bool enum_from_string(std::string_view name, severity& out) {
   if (name == "info") {
      out = severity::info;
      return true;
   }
   if (name == "warning") {
      out = severity::warning;
      return true;
   }
   if (name == "error") {
      out = severity::error;
      return true;
   }
   return false;
}

[[nodiscard]] inline std::optional<std::string> enum_to_string(severity value) {
   switch (value) {
      case severity::info:
         return "info";
      case severity::warning:
         return "warning";
      case severity::error:
         return "error";
   }
   return std::nullopt;
}

[[nodiscard]] inline bool enum_from_int(std::int64_t value, severity& out) {
   if (value == 0) {
      out = severity::info;
      return true;
   }
   if (value == 1) {
      out = severity::warning;
      return true;
   }
   if (value == 2) {
      out = severity::error;
      return true;
   }
   return false;
}

template <typename E>
[[nodiscard]] bool enum_from_string(std::string_view name, E& out) {
   static_assert(std::is_enum_v<E>, "enum_from_string requires an enum type");
   auto matched = false;
   boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>([&](auto descriptor) {
      if (!matched && name == descriptor.name) {
         out = descriptor.value;
         matched = true;
      }
   });
   return matched;
}

template <typename E>
[[nodiscard]] std::optional<std::string> enum_to_string(E value) {
   static_assert(std::is_enum_v<E>, "enum_to_string requires an enum type");
   auto result = std::optional<std::string>{};
   boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>([&](auto descriptor) {
      if (!result && value == descriptor.value) {
         result = descriptor.name;
      }
   });
   return result;
}

template <typename E>
[[nodiscard]] bool enum_from_int(std::int64_t value, E& out) {
   static_assert(std::is_enum_v<E>, "enum_from_int requires an enum type");
   auto matched = false;
   boost::mp11::mp_for_each<boost::describe::describe_enumerators<E>>([&](auto descriptor) {
      if (!matched && static_cast<std::int64_t>(descriptor.value) == value) {
         out = descriptor.value;
         matched = true;
      }
   });
   return matched;
}

} // namespace fcl::schema
