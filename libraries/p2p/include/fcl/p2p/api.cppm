module;

#include <boost/asio/awaitable.hpp>
#include <fcl/exception/macros.hpp>

#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <utility>
#include <vector>

export module fcl.p2p.api;

import fcl.api;
import fcl.p2p.exceptions;
import fcl.p2p.node;
import fcl.p2p.protocol;
import fcl.raw.raw;
import fcl.quic.errors;

export namespace fcl::p2p {

struct api_peer_policy {
   bool require_known_peer = false;
};

struct api_discovery_scope {
   std::string value;
};

class api_binding {
 public:
   using session_handler = std::function<boost::asio::awaitable<void>(fcl::api::session&)>;

   api_binding(node* owner, fcl::api::binding_plan plan, protocol_id protocol, fcl::api::codec_id codec,
               api_peer_policy peer_policy, api_discovery_scope discovery_scope, std::size_t max_inflight_per_peer)
       : owner_{owner}, plan_{std::move(plan)}, protocol_{std::move(protocol)}, codec_{std::move(codec)},
         peer_policy_{std::move(peer_policy)}, discovery_scope_{std::move(discovery_scope)},
         max_inflight_per_peer_{max_inflight_per_peer} {}

   api_binding& on_session(session_handler handler) {
      on_session_ = std::move(handler);
      return *this;
   }

   [[nodiscard]] const protocol_id& protocol() const noexcept {
      return protocol_;
   }

   [[nodiscard]] protocol_handler handler() const {
      return [binding = *this](incoming_protocol_stream stream) mutable -> boost::asio::awaitable<void> {
         co_await binding.accept(std::move(stream));
      };
   }

   boost::asio::awaitable<fcl::api::session> accept(incoming_protocol_stream stream) const {
      auto session = make_session();
      if (on_session_) {
         co_await on_session_(session);
      }
      co_await serve(std::move(stream));
      co_return session;
   }

   boost::asio::awaitable<void> serve(incoming_protocol_stream stream) const {
      validate_stream(stream);
      auto calls = fcl::api::call_runtime{
          fcl::api::call_runtime_options{.max_inflight = max_inflight_per_peer_}};

      while (true) {
         try {
            auto payload = co_await stream.stream.async_read_frame();
            auto request = fcl::raw::unpack<fcl::api::frame>(
                fcl::api::bytes{reinterpret_cast<const char*>(payload.data()),
                                reinterpret_cast<const char*>(payload.data() + payload.size())});
            if (request.codec != codec_) {
               FCL_THROW_EXCEPTION(fcl::api::exceptions::codec_failed, "P2P API frame codec is not accepted",
                                   fcl::exception::ctx("codec", request.codec.value));
            }
            auto responses = co_await plan_.dispatch_many(std::move(request), calls);
            for (const auto& response : responses) {
               auto response_bytes = fcl::raw::pack(response);
               co_await stream.stream.async_write_frame(
                   std::span<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(response_bytes.data()),
                                                 response_bytes.size()});
            }
         } catch (const fcl::quic::quic_error& error) {
            if (error.kind() == fcl::quic::error_kind::connection_closed ||
                error.kind() == fcl::quic::error_kind::stream_closed ||
                error.kind() == fcl::quic::error_kind::stream_reset ||
                error.kind() == fcl::quic::error_kind::canceled) {
               co_return;
            }
            throw;
         }
      }
   }

   [[nodiscard]] const fcl::api::codec_id& codec() const noexcept {
      return codec_;
   }

   [[nodiscard]] const api_peer_policy& peer_policy() const noexcept {
      return peer_policy_;
   }

   [[nodiscard]] const api_discovery_scope& discovery_scope() const noexcept {
      return discovery_scope_;
   }

   [[nodiscard]] std::size_t max_inflight_per_peer() const noexcept {
      return max_inflight_per_peer_;
   }

 private:
   void validate_stream(const incoming_protocol_stream& stream) const {
      if (stream.protocol != protocol_) {
         FCL_THROW_EXCEPTION(fcl::p2p::exceptions::unsupported_protocol, "P2P API binding received wrong protocol",
                             fcl::exception::ctx("protocol", stream.protocol.value));
      }
      if (peer_policy_.require_known_peer) {
         if (owner_ == nullptr || !owner_->peers().find(stream.session.remote_peer).has_value()) {
            FCL_THROW_EXCEPTION(fcl::p2p::exceptions::peer_not_found, "P2P API peer is not known",
                                fcl::exception::ctx("peer", stream.session.remote_peer.value));
         }
      }
   }

   [[nodiscard]] fcl::api::session make_session() const {
      if (plan_.local == nullptr) {
         FCL_THROW_EXCEPTION(fcl::api::exceptions::incompatible_version, "P2P API binding has no local registry");
      }
      return fcl::api::session{fcl::api::view{*plan_.local}};
   }

   node* owner_ = nullptr;
   fcl::api::binding_plan plan_;
   protocol_id protocol_;
   fcl::api::codec_id codec_;
   api_peer_policy peer_policy_{};
   api_discovery_scope discovery_scope_{};
   std::size_t max_inflight_per_peer_ = 64;
   session_handler on_session_;
};

class api_builder {
 public:
   explicit api_builder(node& owner) : owner_{&owner} {}

   api_builder& use(fcl::api::binding_plan plan) {
      plan_ = std::move(plan);
      return *this;
   }

   api_builder& protocol_id(std::string value) {
      protocol_ = fcl::p2p::protocol_id{.value = std::move(value)};
      return *this;
   }

   api_builder& protocol_id(fcl::p2p::protocol_id value) {
      protocol_ = std::move(value);
      return *this;
   }

   api_builder& codec(fcl::api::codec_id value) {
      codec_ = std::move(value);
      return *this;
   }

   api_builder& peer_policy(api_peer_policy value) {
      peer_policy_ = value;
      return *this;
   }

   api_builder& discovery_scope(api_discovery_scope value) {
      discovery_scope_ = std::move(value);
      return *this;
   }

   api_builder& max_inflight_per_peer(std::size_t value) {
      max_inflight_per_peer_ = value;
      return *this;
   }

   [[nodiscard]] api_binding build() {
      return api_binding{owner_, std::move(plan_), std::move(protocol_), std::move(codec_), peer_policy_,
                         std::move(discovery_scope_), max_inflight_per_peer_};
   }

 private:
   node* owner_ = nullptr;
   fcl::api::binding_plan plan_;
   fcl::p2p::protocol_id protocol_{.value = "/fcl/api/1"};
   fcl::api::codec_id codec_{.value = "fcl.raw"};
   api_peer_policy peer_policy_{};
   api_discovery_scope discovery_scope_{};
   std::size_t max_inflight_per_peer_ = 64;
};

[[nodiscard]] inline api_builder api(node& owner) {
   return api_builder{owner};
}

} // namespace fcl::p2p
