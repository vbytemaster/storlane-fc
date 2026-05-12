module;

#include <functional>
#include <vector>

export module fcl.http.middleware;

import fcl.http.route_context;
import fcl.http.types;

export namespace fcl::http {

using route_handler = std::function<response(route_context&)>;
using next_handler = std::function<response()>;
using middleware = std::function<response(route_context&, next_handler)>;
using middleware_list = std::vector<middleware>;

response run_middleware_chain(
   const middleware_list& middlewares,
   route_context& context,
   route_handler terminal);

} // namespace fcl::http
