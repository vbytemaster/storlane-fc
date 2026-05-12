module;

#include <memory>

#include <boost/asio/awaitable.hpp>

export module fcl.quic.listener;

import fcl.asio.runtime;
import fcl.quic.endpoint;
import fcl.quic.options;
export import fcl.quic.connection;

export namespace fcl::quic {

class listener {
public:
   listener(fcl::asio::runtime& runtime, endpoint bind_endpoint, server_options options);
   ~listener();

   listener(const listener&) = delete;
   listener& operator=(const listener&) = delete;

   [[nodiscard]] endpoint local_endpoint() const;
   boost::asio::awaitable<connection> async_accept();
   void stop();

private:
   struct impl;
   std::unique_ptr<impl> impl_;
};

} // namespace fcl::quic
