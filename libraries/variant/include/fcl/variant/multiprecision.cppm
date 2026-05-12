module;
#include <boost/multiprecision/cpp_int.hpp>

export module fcl.variant.multiprecision;

import fcl.variant.value;

export namespace fcl {
template <typename T> void to_variant(const boost::multiprecision::number<T>& n, variant& v) {
   v = n.str();
}

template <typename T> void from_variant(const variant& v, boost::multiprecision::number<T>& n) {
   n = boost::multiprecision::number<T>(v.get_string());
}
} // namespace fcl
