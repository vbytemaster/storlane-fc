module;

#include <boost/asio/awaitable.hpp>

#include <exception>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

module fcl.api.registry;

import fcl.raw.raw;

namespace fcl::api {

namespace {

[[nodiscard]] frame make_response_base(const frame& request, frame_kind kind = frame_kind::response) {
   return frame{
       .kind = kind,
       .id = request.id,
       .api = request.api,
       .method = request.method,
       .meta = request.meta,
       .codec = request.codec,
   };
}

[[nodiscard]] frame make_error_response(const frame& request, error_payload payload) {
   auto response = make_response_base(request, frame_kind::error);
   response.payload = fcl::raw::pack(std::move(payload));
   return response;
}

} // namespace

registry::registry() = default;
registry::~registry() = default;

const descriptor* registry::describe(api_ref requested) const noexcept {
   const auto* entry = find(std::move(requested));
   return entry == nullptr ? nullptr : &entry->descriptor;
}

boost::asio::awaitable<frame> registry::dispatch(frame request) const {
   auto response = make_response_base(request);

   const auto* entry = find(request.api);
   if (entry == nullptr) {
      co_return make_error_response(request, error_payload{
          .error = "incompatible_version",
          .message = "API is not available or version is incompatible",
          .retryable = false,
          .identity =
              {
                  .category = "fcl.api",
                  .code = static_cast<std::uint32_t>(exceptions::code::incompatible_version),
              },
      });
   }

   const auto* method = find_method(entry->descriptor, request.method);
   if (method == nullptr || !method->raw_invoker) {
      co_return make_error_response(request, error_payload{
          .error = "method_not_found",
          .message = "API method is not available",
          .retryable = false,
          .identity =
              {
                  .category = "fcl.api",
                  .code = static_cast<std::uint32_t>(exceptions::code::method_not_found),
              },
      });
   }

   try {
      response.payload = co_await method->raw_invoker(entry->implementation, std::move(request.payload));
      co_return response;
   } catch (const fcl::exception::base& error) {
      response.kind = frame_kind::error;
      response.payload = fcl::raw::pack(make_error_payload(error, find_error(*method, error)));
      co_return response;
   } catch (const std::exception&) {
      response.kind = frame_kind::error;
      response.payload = fcl::raw::pack(make_internal_error_payload());
      co_return response;
   } catch (...) {
      response.kind = frame_kind::error;
      response.payload = fcl::raw::pack(make_internal_error_payload());
      co_return response;
   }
}

boost::asio::awaitable<std::vector<frame>> registry::dispatch_many(frame request) const {
   const auto* entry = find(request.api);
   if (entry == nullptr) {
      co_return std::vector<frame>{make_error_response(
          request, error_payload{
                       .error = "incompatible_version",
                       .message = "API is not available or version is incompatible",
                       .retryable = false,
                       .identity =
                           {
                               .category = "fcl.api",
                               .code = static_cast<std::uint32_t>(exceptions::code::incompatible_version),
                           },
                   })};
   }

   const auto* method = find_method(entry->descriptor, request.method);
   if (method == nullptr) {
      co_return std::vector<frame>{make_error_response(
          request, error_payload{
                       .error = "method_not_found",
                       .message = "API method is not available",
                       .retryable = false,
                       .identity =
                           {
                               .category = "fcl.api",
                               .code = static_cast<std::uint32_t>(exceptions::code::method_not_found),
                           },
                   })};
   }

   if (method->kind != method_kind::server_stream) {
      auto single = co_await dispatch(std::move(request));
      co_return std::vector<frame>{std::move(single)};
   }

   if (!method->raw_stream_invoker) {
      co_return std::vector<frame>{make_error_response(
          request, error_payload{
                       .error = "method_not_found",
                       .message = "API streaming method is not available",
                       .retryable = false,
                       .identity =
                           {
                               .category = "fcl.api",
                               .code = static_cast<std::uint32_t>(exceptions::code::method_not_found),
                           },
                   })};
   }

   try {
      auto items = co_await method->raw_stream_invoker(entry->implementation, std::move(request.payload));
      auto responses = std::vector<frame>{};
      responses.reserve(items.size() + 1);
      for (auto& payload : items) {
         auto item = make_response_base(request, frame_kind::stream_item);
         item.payload = std::move(payload);
         responses.push_back(std::move(item));
      }
      responses.push_back(make_response_base(request, frame_kind::stream_end));
      co_return responses;
   } catch (const fcl::exception::base& error) {
      co_return std::vector<frame>{make_error_response(request, make_error_payload(error, find_error(*method, error)))};
   } catch (const std::exception&) {
      co_return std::vector<frame>{make_error_response(request, make_internal_error_payload())};
   } catch (...) {
      co_return std::vector<frame>{make_error_response(request, make_internal_error_payload())};
   }
}

std::size_t registry::size() const noexcept {
   return entries_.size();
}

void registry::clear() noexcept {
   entries_.clear();
}

std::string registry::key_for(std::string_view id, std::uint16_t major) {
   std::ostringstream out;
   out << id << "/v" << major;
   return out.str();
}

const registry::entry* registry::find(api_ref requested) const noexcept {
   const auto iterator = entries_.find(key_for(requested.id.value, requested.major));
   if (iterator == entries_.end()) {
      return nullptr;
   }
   if (!compatible(iterator->second.descriptor, requested)) {
      return nullptr;
   }
   return &iterator->second;
}

} // namespace fcl::api
