module;
#include <fcl/core/macros.hpp>
#include <stdexcept>
#include <string>

module fcl.raw.datastream;

NO_RETURN void fcl::detail::throw_datastream_range_error(char const* method, size_t len, int64_t over) {
   throw std::out_of_range(std::string(method) + " datastream of length " + std::to_string(len) + " over by " +
                           std::to_string(over));
}
