#include <boost/describe.hpp>
#include <boost/test/unit_test.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

import fcl.config;
import fcl.schema;

namespace {

struct http_config {
   std::uint16_t bind_port = 0;
   std::string bind_host;
   bool tls_enabled = false;
   std::vector<std::string> tags;
   std::string token;
};

struct flat_config {
   std::string log_level;
};

} // namespace

BOOST_DESCRIBE_STRUCT(http_config, (), (bind_port, bind_host, tls_enabled, tags, token))
BOOST_DESCRIBE_STRUCT(flat_config, (), (log_level))

template <> struct fcl::schema::rules<http_config> {
   [[nodiscard]] static fcl::schema::object_schema<http_config> define() {
      auto schema = fcl::schema::object<http_config>();
      schema.field<&http_config::bind_port>("bind-port").alias("port").required().default_value(8080).range(1, 65535);
      schema.field<&http_config::bind_host>("bind-host").default_value("127.0.0.1");
      schema.field<&http_config::tls_enabled>("tls-enabled").default_value(false);
      static_cast<void>(schema.field<&http_config::tags>("tags"));
      schema.field<&http_config::token>("token").secret().deprecated("use vault-ref");
      return schema;
   }
};

template <> struct fcl::schema::rules<flat_config> {
   [[nodiscard]] static fcl::schema::object_schema<flat_config> define() {
      auto schema = fcl::schema::object<flat_config>();
      schema.field<&flat_config::log_level>("log-level").default_value("info");
      return schema;
   }
};

BOOST_AUTO_TEST_CASE(config_key_path_splits_dotted_keys) {
   auto segments = fcl::config::key_path{.value = "http.bind-port"}.segments();
   BOOST_REQUIRE_EQUAL(segments.size(), 2U);
   BOOST_TEST(segments[0] == "http");
   BOOST_TEST(segments[1] == "bind-port");

   auto compacted = fcl::config::key_path{.value = ".http..tls-enabled."}.segments();
   BOOST_REQUIRE_EQUAL(compacted.size(), 2U);
   BOOST_TEST(compacted[0] == "http");
   BOOST_TEST(compacted[1] == "tls-enabled");
}

BOOST_AUTO_TEST_CASE(config_document_paths_merge_and_decode) {
   auto defaults = fcl::config::defaults_for<http_config>("http");
   auto file = fcl::config::document{};
   file.set("http.bind-port", 8081);
   auto cli = fcl::config::document{};
   cli.set("http.bind-port", 9090);
   cli.set("http.tls-enabled", false);
   cli.set("http.tags", fcl::config::value::array_type{fcl::config::value{"alpha"}, fcl::config::value{"beta"}});

   const auto merged = fcl::config::merge({defaults, file, cli});
   const auto decoded = fcl::config::decode<http_config>(merged, "http");
   BOOST_TEST(decoded.ok());
   BOOST_TEST(decoded.value.bind_port == 9090U);
   BOOST_TEST(decoded.value.bind_host == "127.0.0.1");
   BOOST_TEST(!decoded.value.tls_enabled);
   BOOST_REQUIRE_EQUAL(decoded.value.tags.size(), 2U);
   BOOST_TEST(decoded.value.tags[1] == "beta");
}

BOOST_AUTO_TEST_CASE(config_reports_required_unknown_deprecated_and_redacts) {
   auto doc = fcl::config::document{};
   doc.set("http.bind-host", "127.0.0.1");
   doc.set("http.token", "secret-value");
   doc.set("http.extra", "ignored");

   const auto decoded = fcl::config::decode<http_config>(doc, "http");
   BOOST_TEST(!decoded.ok());
   BOOST_TEST(decoded.diagnostics.entries.size() >= 3U);

   auto registry = fcl::config::component_registry{};
   registry.add(fcl::config::describe_component<http_config>("http"));
   auto redacted = fcl::config::redact(doc, registry);
   const auto* token = redacted.try_get("http.token");
   BOOST_REQUIRE(token != nullptr);
   BOOST_TEST(std::get<std::string>(token->storage) == "<redacted>");
}

BOOST_AUTO_TEST_CASE(config_registry_rejects_duplicate_aliases) {
   auto registry = fcl::config::component_registry{};
   registry.add(fcl::config::describe_component<http_config>("http"));
   BOOST_CHECK_THROW(registry.add(fcl::config::describe_component<http_config>("http")), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(config_registry_supports_empty_component_sections) {
   auto registry = fcl::config::component_registry{};
   registry.add(fcl::config::describe_component<flat_config>(""));

   auto doc = fcl::config::document{};
   doc.set("log-level", "debug");

   const auto view = fcl::config::component_view{doc, ""};
   BOOST_TEST(view.get_or<std::string>("log-level", "info") == "debug");

   const auto decoded = fcl::config::decode<flat_config>(doc);
   BOOST_TEST(decoded.ok());
   BOOST_TEST(decoded.value.log_level == "debug");
   BOOST_REQUIRE_EQUAL(registry.components().front().fields.size(), 1U);
   BOOST_TEST(registry.components().front().fields.front().has_default);
}
