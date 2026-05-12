module;

#include <functional>

module fcl.http.middleware;

namespace fcl::http {

response run_middleware_chain(const middleware_list& middlewares, route_context& context, route_handler terminal) {
   try {
      auto invoke = std::function<response(std::size_t)>{};
      invoke = [&](std::size_t index) -> response {
         if (index == middlewares.size()) {
            return terminal(context);
         }
         return middlewares[index](context, [&]() { return invoke(index + 1U); });
      };

      return invoke(0);
   } catch (...) {
      return make_text_response(context.request, status::internal_server_error, "internal server error");
   }
}

} // namespace fcl::http
