#include <boost/test/unit_test.hpp>

#include <boost/asio/awaitable.hpp>

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

import fcl.app;
import fcl.asio.blocking;
import fcl.asio.runtime;
import fcl.asio.task_scheduler;
import fcl.config;
import fcl.schema;

namespace {

struct sample_port {
   virtual ~sample_port() = default;
   virtual int value() const = 0;
};

class sample_port_impl final : public sample_port {
 public:
   explicit sample_port_impl(int value) : value_{value} {}

   int value() const override {
      return value_;
   }

 private:
   int value_ = 0;
};

struct lifecycle_log {
   std::vector<std::string> entries;
};

class test_plugin final : public fcl::app::plugin {
 public:
   test_plugin(std::string id, lifecycle_log& log, bool fail_startup = false)
       : id_{std::move(id)}, log_{&log}, fail_startup_{fail_startup} {}

   fcl::app::plugin_id id() const override {
      return fcl::app::plugin_id{.value = id_};
   }
   std::string version() const override {
      return "1";
   }

   boost::asio::awaitable<void> initialize(fcl::app::plugin_context&) override {
      log_->entries.push_back("initialize:" + id_);
      co_return;
   }

   boost::asio::awaitable<void> startup() override {
      log_->entries.push_back("startup:" + id_);
      if (fail_startup_) {
         throw std::runtime_error{"startup failed"};
      }
      co_return;
   }

   boost::asio::awaitable<void> shutdown() override {
      log_->entries.push_back("shutdown:" + id_);
      co_return;
   }

 private:
   std::string id_;
   lifecycle_log* log_ = nullptr;
   bool fail_startup_ = false;
};

class configurable_plugin final : public fcl::app::plugin {
 public:
   configurable_plugin(lifecycle_log& log) : log_{&log} {}

   fcl::app::plugin_id id() const override {
      return fcl::app::plugin_id{.value = "configurable"};
   }
   std::string version() const override {
      return "1";
   }

   std::optional<fcl::config::component_descriptor> describe_config() const override {
      return fcl::config::component_descriptor{
          .section = "http",
          .fields = {fcl::config::field_descriptor{
              .name = "bind-port",
              .kind = fcl::schema::value_kind::unsigned_integer,
              .required = true,
          }},
      };
   }

   boost::asio::awaitable<void> configure(fcl::config::component_view view) override {
      bind_port_ = view.get_or<std::uint16_t>("bind-port", 0);
      log_->entries.push_back("configure:" + std::to_string(bind_port_));
      co_return;
   }

   boost::asio::awaitable<void> initialize(fcl::app::plugin_context&) override {
      if (bind_port_ == 0) {
         throw std::runtime_error{"not configured"};
      }
      log_->entries.push_back("initialize:configurable");
      co_return;
   }

   boost::asio::awaitable<void> startup() override {
      log_->entries.push_back("startup:configurable");
      co_return;
   }

   boost::asio::awaitable<void> shutdown() override {
      log_->entries.push_back("shutdown:configurable");
      co_return;
   }

 private:
   lifecycle_log* log_ = nullptr;
   std::uint16_t bind_port_ = 0;
};

fcl::app::plugin_descriptor descriptor(std::string id, lifecycle_log& log,
                                       std::vector<fcl::app::plugin_id> dependencies = {},
                                       bool enabled_by_default = true, bool fail_startup = false) {
   auto id_value = fcl::app::plugin_id{.value = std::move(id)};
   auto id_copy = id_value;
   return fcl::app::plugin_descriptor{
       .id = std::move(id_value),
       .dependencies = std::move(dependencies),
       .enabled_by_default = enabled_by_default,
       .factory = [id_copy, &log,
                   fail_startup] { return std::make_unique<test_plugin>(id_copy.value, log, fail_startup); },
   };
}

} // namespace

BOOST_AUTO_TEST_CASE(port_registry_installs_gets_and_rejects_duplicates) {
   auto ports = fcl::app::port_registry{};
   BOOST_TEST(ports.size() == 0);
   BOOST_TEST(!ports.try_get<sample_port>());

   ports.install<sample_port>(std::make_shared<sample_port_impl>(7));
   BOOST_TEST(ports.get<sample_port>()->value() == 7);
   BOOST_CHECK_THROW(ports.install<sample_port>(std::make_shared<sample_port_impl>(8)), fcl::app::port_error);

   ports.remove<sample_port>();
   BOOST_TEST(!ports.try_get<sample_port>());
   BOOST_CHECK_THROW(static_cast<void>(ports.get<sample_port>()), fcl::app::port_error);
}

BOOST_AUTO_TEST_CASE(event_bus_bounds_queues_and_keeps_critical_ring) {
   auto bus = fcl::app::event_bus{fcl::app::event_bus_options{
       .max_subscription_events = 1,
       .max_critical_events = 2,
       .max_recent_events = 3,
   }};
   auto subscription = bus.subscribe(fcl::app::event_filter{.topic = "app"});

   bus.publish(fcl::app::event_severity::info, "app.start", "first");
   bus.publish(fcl::app::event_severity::info, "app.start", "dropped");
   bus.publish(fcl::app::event_severity::critical, "system", "critical-a");
   bus.publish(fcl::app::event_severity::critical, "system", "critical-b");
   bus.publish(fcl::app::event_severity::critical, "system", "critical-c");

   BOOST_TEST(bus.metrics().dropped == 1);
   auto first = subscription.poll();
   BOOST_REQUIRE(first.has_value());
   BOOST_TEST(first->message == "first");
   BOOST_TEST(!subscription.poll().has_value());

   const auto critical = bus.critical_events();
   BOOST_REQUIRE_EQUAL(critical.size(), 2);
   BOOST_TEST(critical.front().message == "critical-b");
   BOOST_TEST(critical.back().message == "critical-c");

   const auto recent = bus.recent_events();
   BOOST_REQUIRE_EQUAL(recent.size(), 3);
   BOOST_TEST(recent.front().message == "critical-a");
   BOOST_TEST(recent.back().message == "critical-c");
}

BOOST_AUTO_TEST_CASE(plugin_registry_orders_dependencies_and_rejects_bad_graphs) {
   auto log = lifecycle_log{};
   auto registry = fcl::app::plugin_registry{};
   registry.register_plugin(descriptor("store", log, {}, false));
   registry.register_plugin(descriptor("provider", log, {fcl::app::plugin_id{.value = "store"}}));

   auto plugins = registry.instantiate_enabled({});
   BOOST_REQUIRE_EQUAL(plugins.size(), 2);
   BOOST_TEST(plugins[0]->id().value == "store");
   BOOST_TEST(plugins[1]->id().value == "provider");

   BOOST_CHECK_THROW(registry.register_plugin(descriptor("store", log)), std::invalid_argument);

   auto broken = fcl::app::plugin_registry{};
   broken.register_plugin(descriptor("broken", log, {fcl::app::plugin_id{.value = "missing"}}));
   BOOST_CHECK_THROW(static_cast<void>(broken.instantiate_enabled({})), std::logic_error);

   BOOST_CHECK_THROW(static_cast<void>(registry.instantiate_enabled({fcl::app::plugin_config{
                         .id = fcl::app::plugin_id{.value = "store"},
                         .enabled = false,
                     }})),
                     std::logic_error);
}

BOOST_AUTO_TEST_CASE(application_runtime_rolls_back_and_shutdown_is_idempotent) {
   auto runtime = fcl::asio::runtime{};
   auto scheduler = fcl::asio::task_scheduler{runtime};
   auto ports = fcl::app::port_registry{};
   auto signals = fcl::app::signal_bus{};
   auto events = fcl::app::event_bus{};
   auto context = fcl::app::plugin_context{scheduler, ports, signals, events};
   auto log = lifecycle_log{};

   auto registry = fcl::app::plugin_registry{};
   registry.register_plugin(descriptor("a", log));
   registry.register_plugin(descriptor("b", log, {}, true, true));

   auto app = fcl::app::application_runtime{context, registry.instantiate_enabled({})};
   BOOST_CHECK_THROW(fcl::asio::blocking::run(runtime, app.startup()), std::runtime_error);
   BOOST_CHECK(app.state() == fcl::app::application_state::stopped);
   BOOST_REQUIRE_EQUAL(log.entries.size(), 6);
   BOOST_TEST(log.entries[0] == "initialize:a");
   BOOST_TEST(log.entries[1] == "initialize:b");
   BOOST_TEST(log.entries[2] == "startup:a");
   BOOST_TEST(log.entries[3] == "startup:b");
   BOOST_TEST(log.entries[4] == "shutdown:b");
   BOOST_TEST(log.entries[5] == "shutdown:a");

   fcl::asio::blocking::run(runtime, app.shutdown());
   fcl::asio::blocking::run(runtime, app.shutdown());
}

BOOST_AUTO_TEST_CASE(application_runtime_rejects_startup_after_shutdown) {
   auto runtime = fcl::asio::runtime{};
   auto scheduler = fcl::asio::task_scheduler{runtime};
   auto ports = fcl::app::port_registry{};
   auto signals = fcl::app::signal_bus{};
   auto events = fcl::app::event_bus{};
   auto context = fcl::app::plugin_context{scheduler, ports, signals, events};
   auto log = lifecycle_log{};

   auto registry = fcl::app::plugin_registry{};
   registry.register_plugin(descriptor("a", log));

   auto app = fcl::app::application_runtime{context, registry.instantiate_enabled({})};
   fcl::asio::blocking::run(runtime, app.startup());
   fcl::asio::blocking::run(runtime, app.shutdown());

   BOOST_CHECK(app.state() == fcl::app::application_state::stopped);
   BOOST_REQUIRE_EQUAL(log.entries.size(), 3);
   BOOST_TEST(log.entries[0] == "initialize:a");
   BOOST_TEST(log.entries[1] == "startup:a");
   BOOST_TEST(log.entries[2] == "shutdown:a");

   BOOST_CHECK_THROW(fcl::asio::blocking::run(runtime, app.startup()), std::logic_error);
   BOOST_CHECK(app.state() == fcl::app::application_state::stopped);
   BOOST_REQUIRE_EQUAL(log.entries.size(), 3);
}

BOOST_AUTO_TEST_CASE(application_runtime_records_diagnostics_and_events) {
   auto runtime = fcl::asio::runtime{};
   auto scheduler = fcl::asio::task_scheduler{runtime};
   auto ports = fcl::app::port_registry{};
   auto signals = fcl::app::signal_bus{};
   auto events = fcl::app::event_bus{};
   auto diagnostics = fcl::app::diagnostics_store{};
   auto context = fcl::app::plugin_context{scheduler, ports, signals, events, &diagnostics};
   auto log = lifecycle_log{};

   auto registry = fcl::app::plugin_registry{};
   registry.register_plugin(descriptor("a", log));

   auto app = fcl::app::application_runtime{context, registry.instantiate_enabled({}), &diagnostics};
   fcl::asio::blocking::run(runtime, app.startup());

   auto snapshot = diagnostics.snapshot(events);
   BOOST_REQUIRE_EQUAL(snapshot.plugins.size(), 1U);
   BOOST_TEST(snapshot.plugins.front().id == "a");
   BOOST_TEST(static_cast<int>(snapshot.plugins.front().state) == static_cast<int>(fcl::app::lifecycle_state::started));
   BOOST_TEST(snapshot.events.published >= 4U);
   BOOST_TEST(!snapshot.recent_events.empty());

   fcl::asio::blocking::run(runtime, app.shutdown());
   snapshot = diagnostics.snapshot(events);
   BOOST_TEST(static_cast<int>(snapshot.plugins.front().state) == static_cast<int>(fcl::app::lifecycle_state::stopped));
}

BOOST_AUTO_TEST_CASE(application_runtime_records_failed_plugin_diagnostics) {
   auto runtime = fcl::asio::runtime{};
   auto scheduler = fcl::asio::task_scheduler{runtime};
   auto ports = fcl::app::port_registry{};
   auto signals = fcl::app::signal_bus{};
   auto events = fcl::app::event_bus{};
   auto diagnostics = fcl::app::diagnostics_store{};
   auto context = fcl::app::plugin_context{scheduler, ports, signals, events, &diagnostics};
   auto log = lifecycle_log{};

   auto registry = fcl::app::plugin_registry{};
   registry.register_plugin(descriptor("a", log, {}, true, true));

   auto app = fcl::app::application_runtime{context, registry.instantiate_enabled({}), &diagnostics};
   BOOST_CHECK_THROW(fcl::asio::blocking::run(runtime, app.startup()), std::runtime_error);

   const auto snapshot = diagnostics.snapshot(events);
   BOOST_TEST(static_cast<int>(snapshot.state) == static_cast<int>(fcl::app::lifecycle_state::failed));
   BOOST_REQUIRE_EQUAL(snapshot.plugins.size(), 1U);
   BOOST_TEST(static_cast<int>(snapshot.plugins.front().state) == static_cast<int>(fcl::app::lifecycle_state::failed));
   BOOST_TEST(snapshot.plugins.front().last_error == "startup failed");
}

BOOST_AUTO_TEST_CASE(application_runtime_collects_and_applies_plugin_config_before_initialize) {
   auto runtime = fcl::asio::runtime{};
   auto scheduler = fcl::asio::task_scheduler{runtime};
   auto ports = fcl::app::port_registry{};
   auto signals = fcl::app::signal_bus{};
   auto events = fcl::app::event_bus{};
   auto context = fcl::app::plugin_context{scheduler, ports, signals, events};
   auto log = lifecycle_log{};

   auto plugins = std::vector<std::unique_ptr<fcl::app::plugin>>{};
   plugins.push_back(std::make_unique<configurable_plugin>(log));

   auto app = fcl::app::application_runtime{context, std::move(plugins)};
   const auto registry = app.describe_config();
   BOOST_REQUIRE_EQUAL(registry.components().size(), 1U);
   BOOST_TEST(registry.components().front().section == "http");

   auto document = fcl::config::document{};
   document.set("http.bind-port", 7777);
   fcl::asio::blocking::run(runtime, app.configure(document));
   fcl::asio::blocking::run(runtime, app.startup());
   fcl::asio::blocking::run(runtime, app.shutdown());

   BOOST_REQUIRE_EQUAL(log.entries.size(), 4U);
   BOOST_TEST(log.entries[0] == "configure:7777");
   BOOST_TEST(log.entries[1] == "initialize:configurable");
   BOOST_TEST(log.entries[2] == "startup:configurable");
   BOOST_TEST(log.entries[3] == "shutdown:configurable");
}
