#pragma once

#include "quic_engine.hpp"

#include <memory>

namespace fcl::quic::detail {

struct connection_handle {
   std::shared_ptr<engine_connection> engine;
};

struct stream_handle {
   std::shared_ptr<engine_stream> engine;
};

} // namespace fcl::quic::detail
