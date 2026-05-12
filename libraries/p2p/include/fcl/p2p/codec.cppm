module;

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include <boost/asio/awaitable.hpp>

export module fcl.p2p.codec;

import fcl.p2p.message;
import fcl.quic.framed_stream;

export namespace fcl::p2p {

struct codec_options {
   std::uint32_t max_message_size = 4 * 1024 * 1024;
   std::uint32_t max_endpoint_records = 1024;
};

[[nodiscard]] std::vector<std::uint8_t> encode_message(const p2p_message& message, codec_options options = {});
[[nodiscard]] p2p_message decode_message(std::span<const std::uint8_t> bytes, codec_options options = {});

boost::asio::awaitable<void> async_write_message(fcl::quic::framed_stream& stream, const p2p_message& message,
                                                 codec_options options = {});

boost::asio::awaitable<p2p_message> async_read_message(fcl::quic::framed_stream& stream, codec_options options = {});

} // namespace fcl::p2p
