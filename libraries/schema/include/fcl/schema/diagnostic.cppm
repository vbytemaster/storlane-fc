module;

#include <string>

export module fcl.schema.diagnostic;

export namespace fcl::schema {

enum class severity {
   info,
   warning,
   error,
};

struct diagnostic {
   std::string path;
   std::string code;
   severity level = severity::error;
   std::string message;
};

} // namespace fcl::schema
