#include <fcl/io/datastream.hpp>
#include <fcl/exception/exception.hpp>

NO_RETURN void fcl::detail::throw_datastream_range_error(char const* method, size_t len, int64_t over)
{
  FCL_THROW_EXCEPTION( out_of_range_exception, "${method} datastream of length ${len} over by ${over}", ("method",std::string(method))("len",len)("over",over) );
}
