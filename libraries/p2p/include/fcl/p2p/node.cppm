module;

#include <chrono>
#include <functional>
#include <memory>
#include <optional>

#include <boost/asio/awaitable.hpp>

export module fcl.p2p.node;

import fcl.asio.runtime;
import fcl.p2p.identity;
import fcl.p2p.metrics;
import fcl.p2p.options;
import fcl.p2p.peer_store;
import fcl.p2p.protocol;
import fcl.p2p.relay;
import fcl.p2p.session;
import fcl.quic.endpoint;
import fcl.quic.framed_stream;

export namespace fcl::p2p {

struct incoming_protocol_stream {
   session_info session;
   protocol_id protocol;
   fcl::quic::framed_stream stream;
};

using protocol_handler = std::function<boost::asio::awaitable<void>(incoming_protocol_stream)>;

class node {
 public:
   node(fcl::asio::runtime& runtime, node_options options);
   ~node();

   node(const node&) = delete;
   node& operator=(const node&) = delete;

   node(node&&) noexcept;
   node& operator=(node&&) noexcept;

   [[nodiscard]] const peer_id& local_peer() const noexcept;
   [[nodiscard]] std::optional<fcl::quic::endpoint> local_endpoint() const;
   [[nodiscard]] node_metrics metrics() const;
   [[nodiscard]] peer_store& peers() noexcept;
   [[nodiscard]] const peer_store& peers() const noexcept;

   void register_protocol_handler(protocol_id protocol, protocol_handler handler);
   boost::asio::awaitable<void> async_listen(fcl::quic::endpoint endpoint);
   boost::asio::awaitable<session_info> async_connect(fcl::quic::endpoint endpoint, connect_options options = {});
   boost::asio::awaitable<void> async_request_peer_exchange(peer_id peer);
   boost::asio::awaitable<reachability_state> async_probe_reachability(peer_id observer);
   boost::asio::awaitable<relay_reservation_info> async_reserve_relay(peer_id relay_peer,
                                                                      relay_reservation_options options = {});
   boost::asio::awaitable<void> async_cancel_relay(peer_id relay_peer);
   boost::asio::awaitable<hole_punch_status>
   async_attempt_hole_punch(peer_id peer, std::optional<peer_id> relay_peer = std::nullopt,
                            std::chrono::milliseconds timeout = std::chrono::milliseconds{10'000});
   boost::asio::awaitable<fcl::quic::framed_stream> async_open_protocol_stream(peer_id peer, protocol_id protocol,
                                                                               open_options options = {});
   boost::asio::awaitable<void> async_stop();
   void stop();

 private:
   struct impl;
   std::shared_ptr<impl> impl_;
};

} // namespace fcl::p2p
