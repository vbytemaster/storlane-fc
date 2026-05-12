module;
#include <boost/dynamic_bitset.hpp>
#include <cstdint>

export module fcl.variant.dynamic_bitset;

export namespace fcl {

using dynamic_bitset = boost::dynamic_bitset<std::uint8_t>;

} // namespace fcl

