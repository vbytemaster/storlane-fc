module;
#include <string>

export module fcl.variant.format;

import fcl.variant.value;

export namespace fcl {
std::string format_string(const std::string& format, const variant_object& args, bool minimize = false);
}
