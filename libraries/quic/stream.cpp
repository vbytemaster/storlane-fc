module;

#include "wrapper_handles.hpp"

#include <cstdint>
#include <memory>
#include <span>
#include <utility>
#include <vector>

#include <boost/asio/awaitable.hpp>

module fcl.quic.stream;

import fcl.quic.errors;

namespace fcl::quic {
namespace {

[[nodiscard]] error_kind map_error(detail::engine_error_kind kind) noexcept {
   switch (kind) {
   case detail::engine_error_kind::invalid_endpoint:
      return error_kind::invalid_endpoint;
   case detail::engine_error_kind::invalid_options:
      return error_kind::invalid_options;
   case detail::engine_error_kind::dependency_unavailable:
      return error_kind::dependency_unavailable;
   case detail::engine_error_kind::connect_timeout:
      return error_kind::connect_timeout;
   case detail::engine_error_kind::handshake_timeout:
      return error_kind::handshake_timeout;
   case detail::engine_error_kind::idle_timeout:
      return error_kind::idle_timeout;
   case detail::engine_error_kind::tls_failed:
      return error_kind::tls_failed;
   case detail::engine_error_kind::peer_verification_failed:
      return error_kind::peer_verification_failed;
   case detail::engine_error_kind::alpn_mismatch:
      return error_kind::alpn_mismatch;
   case detail::engine_error_kind::frame_too_large:
      return error_kind::frame_too_large;
   case detail::engine_error_kind::malformed_frame:
      return error_kind::malformed_frame;
   case detail::engine_error_kind::backpressure_rejected:
      return error_kind::backpressure_rejected;
   case detail::engine_error_kind::connection_closed:
      return error_kind::connection_closed;
   case detail::engine_error_kind::stream_closed:
      return error_kind::stream_closed;
   case detail::engine_error_kind::stream_reset:
      return error_kind::stream_reset;
   case detail::engine_error_kind::canceled:
      return error_kind::canceled;
   case detail::engine_error_kind::internal_error:
      return error_kind::internal_error;
   }
   return error_kind::internal_error;
}

[[noreturn]] void rethrow_engine_error(const detail::engine_error& error) {
   throw_quic_error(map_error(error.kind()), error.what());
}

} // namespace

struct stream::impl {
   std::shared_ptr<detail::engine_stream> engine;
};

stream::stream() = default;

stream::stream(detail::stream_handle handle)
    : impl_(std::make_shared<impl>(impl{.engine = std::move(handle.engine)})) {}

stream::~stream() = default;

stream::stream(stream&&) noexcept = default;
stream& stream::operator=(stream&&) noexcept = default;

bool stream::valid() const noexcept {
   return impl_ != nullptr;
}

std::int64_t stream::id() const noexcept {
   return impl_ && impl_->engine ? impl_->engine->id() : -1;
}

boost::asio::awaitable<void> stream::async_write(std::span<const std::uint8_t> bytes) {
   if (!impl_ || !impl_->engine) {
      throw_quic_error(error_kind::stream_closed, "invalid QUIC stream");
   }
   try {
      co_await impl_->engine->async_write(bytes);
   } catch (const detail::engine_error& error) {
      rethrow_engine_error(error);
   }
   co_return;
}

boost::asio::awaitable<std::vector<std::uint8_t>> stream::async_read() {
   if (!impl_ || !impl_->engine) {
      throw_quic_error(error_kind::stream_closed, "invalid QUIC stream");
   }
   try {
      co_return co_await impl_->engine->async_read();
   } catch (const detail::engine_error& error) {
      rethrow_engine_error(error);
   }
}

boost::asio::awaitable<void> stream::async_close() {
   if (impl_ && impl_->engine) {
      try {
         co_await impl_->engine->async_close();
      } catch (const detail::engine_error& error) {
         rethrow_engine_error(error);
      }
   }
   co_return;
}

stream detail::stream_access::make(detail::stream_handle handle) {
   return stream{std::move(handle)};
}

} // namespace fcl::quic
