module;

#include <optional>

export module fcl.p2p.session;

import fcl.p2p.identity;
import fcl.p2p.protocol;
import fcl.p2p.scoring;

export namespace fcl::p2p {

struct session_info {
   peer_id remote_peer;
   capability_set capabilities{};
   path_kind path = path_kind::direct;
   std::optional<peer_id> relay_peer;
};

} // namespace fcl::p2p
