module;

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

module fcl.config.document;

import fcl.config.key_path;
import fcl.config.value;

namespace fcl::config {

void document::set(std::string dotted_path, value input, source_location location) {
   auto segments = key_path{.value = dotted_path}.segments();
   if (segments.empty()) {
      throw std::invalid_argument{"config key path must not be empty"};
   }

   auto* object = &root;
   for (std::size_t i = 0; i + 1 < segments.size(); ++i) {
      auto& child = (*object)[segments[i]];
      if (!child.is_object()) {
         child = value::object_type{};
      }
      object = child.as_object();
   }
   (*object)[segments.back()] = std::move(input);
   locations[std::move(dotted_path)] = std::move(location);
}

const value* document::try_get(std::string_view dotted_path) const {
   auto segments = key_path{.value = std::string{dotted_path}}.segments();
   if (segments.empty()) {
      return nullptr;
   }

   const auto* object = &root;
   for (std::size_t i = 0; i < segments.size(); ++i) {
      const auto found = object->find(segments[i]);
      if (found == object->end()) {
         return nullptr;
      }
      if (i + 1 == segments.size()) {
         return &found->second;
      }
      object = found->second.as_object();
      if (!object) {
         return nullptr;
      }
   }
   return nullptr;
}

const value::object_type* document::object_at(std::string_view dotted_path) const {
   if (dotted_path.empty()) {
      return &root;
   }
   const auto* entry = try_get(dotted_path);
   return entry ? entry->as_object() : nullptr;
}

document merge(std::initializer_list<document> layers) {
   auto result = document{};
   auto merge_object = [](auto& self, value::object_type& target, const value::object_type& source) -> void {
      for (const auto& [name, input] : source) {
         if (auto* input_object = input.as_object()) {
            auto& target_entry = target[name];
            if (!target_entry.is_object()) {
               target_entry = value::object_type{};
            }
            self(self, *target_entry.as_object(), *input_object);
         } else {
            target[name] = input;
         }
      }
   };

   for (const auto& layer : layers) {
      merge_object(merge_object, result.root, layer.root);
      for (const auto& [path, location] : layer.locations) {
         result.locations.insert_or_assign(path, location);
      }
   }
   return result;
}

document effective_document(std::initializer_list<document> layers) {
   return merge(layers);
}

} // namespace fcl::config
