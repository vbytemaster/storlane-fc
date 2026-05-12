module;

#include <string>
#include <vector>

export module fcl.config.key_path;

export namespace fcl::config {

struct key_path {
   std::string value;

   [[nodiscard]] std::vector<std::string> segments() const;
};

} // namespace fcl::config
