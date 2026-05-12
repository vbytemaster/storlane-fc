module;

#include <string>
#include <utility>

#include <boost/beast/http.hpp>

export module fcl.http.types;

export namespace fcl::http {

using field = boost::beast::http::field;
using method = boost::beast::http::verb;
using status = boost::beast::http::status;
using string_body = boost::beast::http::string_body;
using request = boost::beast::http::request<string_body>;
using response = boost::beast::http::response<string_body>;

inline response make_text_response(const request& source, status result, std::string body,
                                   std::string content_type = "text/plain") {
   auto reply = response{result, source.version()};
   reply.set(field::content_type, std::move(content_type));
   reply.body() = std::move(body);
   reply.prepare_payload();
   reply.keep_alive(source.keep_alive());
   return reply;
}

} // namespace fcl::http
