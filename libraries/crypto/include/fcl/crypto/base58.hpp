#pragma once
#include <fcl/core/string.hpp>
#include <fcl/core/time.hpp>
#include <vector>

namespace fcl {
    std::string to_base58( const char* d, size_t s, const fcl::yield_function_t& yield );
    std::string to_base58( const std::vector<char>& data, const fcl::yield_function_t& yield );
    std::vector<char> from_base58( const std::string& base58_str );
    size_t from_base58( const std::string& base58_str, char* out_data, size_t out_data_len );
}
