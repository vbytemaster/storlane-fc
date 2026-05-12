module;

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

module fcl.http.router;

import fcl.http.target;

namespace fcl::http {
namespace {

std::vector<std::string> split_route_path(const std::string& path) {
   if (path.empty() || path.front() != '/') {
      throw std::invalid_argument{"route path must start with /"};
   }
   if (path == "/") {
      return {};
   }

   auto segments = std::vector<std::string>{};
   auto start = std::size_t{1};
   while (start <= path.size()) {
      const auto separator = path.find('/', start);
      const auto end = separator == std::string::npos ? path.size() : separator;
      segments.push_back(path.substr(start, end - start));
      if (separator == std::string::npos) {
         break;
      }
      start = separator + 1U;
   }
   return segments;
}

bool parameter_segment(const std::string& segment) {
   return segment.size() > 1U && segment.front() == ':';
}

bool parameterized(const std::vector<std::string>& segments) {
   for (const auto& segment : segments) {
      if (parameter_segment(segment)) {
         return true;
      }
   }
   return false;
}

template <typename Entry>
bool match_path(const Entry& entry, const target& parsed_target, std::unordered_map<std::string, std::string>* params) {
   if (entry.segments.size() != parsed_target.segments.size()) {
      return false;
   }

   auto captured = std::unordered_map<std::string, std::string>{};
   for (auto index = std::size_t{0}; index != entry.segments.size(); ++index) {
      const auto& pattern = entry.segments[index];
      const auto& value = parsed_target.segments[index];
      if (parameter_segment(pattern)) {
         captured.emplace(pattern.substr(1), value);
         continue;
      }
      if (pattern != value) {
         return false;
      }
   }

   if (params != nullptr) {
      *params = std::move(captured);
   }
   return true;
}

template <typename Entry>
bool path_exists(const std::vector<Entry>& entries, const target& parsed_target) {
   for (const auto& entry : entries) {
      if (match_path(entry, parsed_target, nullptr)) {
         return true;
      }
   }
   return false;
}

template <typename Entry>
const Entry* find_path_match(
   const std::vector<Entry>& entries,
   const target& parsed_target,
   std::unordered_map<std::string, std::string>& params) {
   for (const auto prefer_parameterized : {false, true}) {
      for (const auto& entry : entries) {
         if (entry.parameterized != prefer_parameterized) {
            continue;
         }
         if (match_path(entry, parsed_target, &params)) {
            return &entry;
         }
      }
   }
   return nullptr;
}

} // namespace

void router::use(middleware handler) {
   middlewares_.push_back(std::move(handler));
}

void router::get(std::string path, route_handler handler) {
   add_route(method::get, std::move(path), std::move(handler));
}

void router::post(std::string path, route_handler handler) {
   add_route(method::post, std::move(path), std::move(handler));
}

void router::put(std::string path, route_handler handler) {
   add_route(method::put, std::move(path), std::move(handler));
}

void router::del(std::string path, route_handler handler) {
   add_route(method::delete_, std::move(path), std::move(handler));
}

void router::websocket(std::string path, websocket_route_handler handler) {
   auto segments = split_route_path(path);
   websocket_routes_.push_back(websocket_route_entry{
      .path = std::move(path),
      .segments = segments,
      .parameterized = parameterized(segments),
      .handler = std::move(handler),
   });
}

response router::handle(route_context& context) const {
   try {
      auto params = std::unordered_map<std::string, std::string>{};
      for (const auto prefer_parameterized : {false, true}) {
         for (const auto& route : routes_) {
            if (route.verb != context.request.method() || route.parameterized != prefer_parameterized) {
               continue;
            }
            if (!match_path(route, context.parsed_target, &params)) {
               continue;
            }

            context.route_params = std::move(params);
            return run_middleware_chain(middlewares_, context, route.handler);
         }
      }

      if (path_exists(routes_, context.parsed_target)) {
         return make_text_response(context.request, status::method_not_allowed, "method not allowed");
      }
      if (path_exists(websocket_routes_, context.parsed_target)) {
         return make_text_response(context.request, status::upgrade_required, "websocket upgrade required");
      }
      return make_text_response(context.request, status::not_found, "not found");
   } catch (...) {
      return make_text_response(context.request, status::internal_server_error, "internal server error");
   }
}

std::optional<websocket_route_handler> router::match_websocket(route_context& context) const {
   if (context.request.method() != method::get) {
      return std::nullopt;
   }

   auto params = std::unordered_map<std::string, std::string>{};
   if (const auto* route = find_path_match(websocket_routes_, context.parsed_target, params); route != nullptr) {
      context.route_params = std::move(params);
      return route->handler;
   }
   return std::nullopt;
}

void router::add_route(method verb, std::string path, route_handler handler) {
   auto segments = split_route_path(path);
   routes_.push_back(route_entry{
      .verb = verb,
      .path = std::move(path),
      .segments = segments,
      .parameterized = parameterized(segments),
      .handler = std::move(handler),
   });
}

} // namespace fcl::http
