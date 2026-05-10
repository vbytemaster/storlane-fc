#include <fstream>
#include <sstream>

#include <fcl/exception/exception.hpp>
#include <fcl/log/logger.hpp>
#include <filesystem>

namespace fcl {

   void read_file_contents( const std::filesystem::path& filename, std::string& result )
   {
      std::ifstream f( filename.string(), std::ios::in | std::ios::binary );
      FCL_ASSERT(f, "Failed to open ${filename}", ("filename", filename.string()));
      // don't use fcl::stringstream here as we need something with override for << rdbuf()
      std::stringstream ss;
      ss << f.rdbuf();
      FCL_ASSERT(f, "Failed reading ${filename}", ("filename", filename.string()));
      result = ss.str();
   }

} // namespace fcl
