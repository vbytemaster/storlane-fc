module;

#include <coroutine>
#include <string>
#include <string_view>
#include <utility>

#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>

module fcl.http.client;

namespace fcl::http {
namespace {

request make_request(method method_value, const base_url& endpoint, std::string_view path, std::string body = {}) {
   auto request_value = request{};
   request_value.method(method_value);
   request_value.target(endpoint.make_target(path));
   request_value.version(11);
   request_value.body() = std::move(body);
   if (!request_value.body().empty()) {
      request_value.set(field::content_type, "application/json");
      request_value.prepare_payload();
   }
   return request_value;
}

} // namespace

client::client(fcl::asio::runtime& runtime, base_url endpoint)
   : endpoint_(std::move(endpoint))
   , connection_(runtime, endpoint_) {}

client::~client() = default;

boost::asio::awaitable<response> client::async_request(fcl::http::request request_value, request_options options) {
   co_return co_await connection_.async_request(std::move(request_value), options);
}

boost::asio::awaitable<response> client::async_get(std::string_view path, request_options options) {
   co_return co_await async_request(make_request(method::get, endpoint_, path), options);
}

boost::asio::awaitable<response> client::async_post_json(
   std::string_view path,
   std::string body,
   request_options options) {
   co_return co_await async_request(make_request(method::post, endpoint_, path, std::move(body)), options);
}

response client::request(fcl::http::request request_value, request_options options) {
   return connection_.request(std::move(request_value), options);
}

response client::get(std::string_view path, request_options options) {
   return request(make_request(method::get, endpoint_, path), options);
}

response client::post_json(std::string_view path, std::string body, request_options options) {
   return request(make_request(method::post, endpoint_, path, std::move(body)), options);
}

connection_metrics client::metrics() const {
   return connection_.metrics();
}

} // namespace fcl::http
