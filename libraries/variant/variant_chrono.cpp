module;
#include <chrono>

module fcl.variant.chrono;

import fcl.core.chrono;
import fcl.variant.value;

namespace fcl {
void to_variant(const std::chrono::sys_time<std::chrono::microseconds>& t, variant& v) {
   v = fcl::chrono::to_iso_string(t);
}

void from_variant(const variant& v, std::chrono::sys_time<std::chrono::microseconds>& t) {
   t = fcl::chrono::from_iso_time_point(v.as_string());
}

void to_variant(const std::chrono::sys_seconds& t, variant& v) {
   v = fcl::chrono::to_iso_string(t);
}

void from_variant(const variant& v, std::chrono::sys_seconds& t) {
   t = fcl::chrono::from_iso_seconds(v.as_string());
}

void to_variant(const std::chrono::microseconds& input_microseconds, variant& output_variant) {
   output_variant = input_microseconds.count();
}

void from_variant(const variant& input_variant, std::chrono::microseconds& output_microseconds) {
   output_microseconds = std::chrono::microseconds{input_variant.as_int64()};
}
} // namespace fcl
