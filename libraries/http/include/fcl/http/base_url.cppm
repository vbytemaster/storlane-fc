module;

#include <cstdint>
#include <string>
#include <string_view>

export module fcl.http.base_url;

export namespace fcl::http {

struct base_url {
   std::string original;
   std::string scheme;
   std::string host;
   std::string port;
   std::string base_path = "/";

   [[nodiscard]] bool secure() const;
   [[nodiscard]] std::string origin() const;
   [[nodiscard]] std::string make_target(std::string_view path) const;
};

base_url parse_base_url(std::string_view value);

} // namespace fcl::http
