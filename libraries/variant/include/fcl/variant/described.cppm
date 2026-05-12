module;
#include <optional>
#include <type_traits>
#include <utility>

export module fcl.variant.described;

import fcl.reflect.reflect;
import fcl.variant.value;
import fcl.variant.conversion;
import fcl.variant.containers;
import fcl.variant.chrono;
import fcl.variant.multiprecision;

export namespace fcl {

template <typename T>
void to_variant(const T& o, variant& v)
   requires fcl::reflect::is_described_enum_v<T>
{
   v = fcl::reflect::enum_to_fc_string(o);
}

template <typename T>
void from_variant(const variant& v, T& o)
   requires fcl::reflect::is_described_enum_v<T>
{
   if (v.is_string()) {
      o = fcl::reflect::enum_from_string<std::remove_const_t<T>>(v.get_string().c_str());
   } else {
      o = fcl::reflect::enum_from_int<std::remove_const_t<T>>(v.as_int64());
   }
}

namespace detail {
template <typename M>
void add_described_member(mutable_variant_object& object, const char* name, const std::optional<M>& value) {
   if (value) {
      object(name, *value);
   }
}

template <typename M> void add_described_member(mutable_variant_object& object, const char* name, const M& value) {
   object(name, value);
}
} // namespace detail

template <typename T>
void to_variant(const T& o, variant& v)
   requires fcl::reflect::is_described_object_v<T>
{
   mutable_variant_object object;
   fcl::reflect::for_each_member<T>(
       [&](const char* name, auto member) { detail::add_described_member(object, name, o.*member); });
   v = std::move(object);
}

template <typename T>
void from_variant(const variant& v, T& o)
   requires fcl::reflect::is_described_object_v<T>
{
   const variant_object& object = v.get_object();
   fcl::reflect::for_each_member<std::remove_const_t<T>>([&](const char* name, auto member) {
      auto itr = object.find(name);
      if (itr != object.end()) {
         using member_type = std::remove_reference_t<decltype(o.*member)>;
         from_variant(itr->value(), const_cast<std::remove_const_t<member_type>&>(o.*member));
      }
   });
}

} // namespace fcl
