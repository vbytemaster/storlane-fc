module;

module fcl.p2p.options;

import fcl.p2p.errors;

namespace fcl::p2p {

void validate(const node_options& options) {
   if (!options.allow_insecure_test_mode && (options.certificate_pem.empty() || options.private_key_pem.empty())) {
      throw_p2p_error(error_kind::invalid_options, "production P2P node requires mTLS certificate and private key");
   }
   if (options.certificate_pem.empty() != options.private_key_pem.empty()) {
      throw_p2p_error(error_kind::invalid_options, "P2P certificate and private key must be provided together");
   }
   if (options.explicit_peer_id && !valid_peer_id(*options.explicit_peer_id)) {
      throw_p2p_error(error_kind::invalid_options, "invalid explicit P2P peer id");
   }
   if (options.allow_insecure_test_mode && options.certificate_pem.empty() && !options.explicit_peer_id) {
      throw_p2p_error(error_kind::invalid_options,
                      "insecure P2P test node without certificate requires explicit peer id");
   }
   if (options.limits.max_sessions == 0 || options.limits.max_protocol_handlers == 0 ||
       options.limits.max_control_message_size == 0 || options.limits.max_peer_exchange_records == 0 ||
       options.limits.max_control_queue == 0 || options.limits.relay.max_active_relays == 0 ||
       options.limits.relay.max_reservations == 0 || options.limits.relay.max_streams_per_reservation == 0 ||
       options.limits.relay.max_relay_bytes == 0 || options.limits.relay.max_queued_bytes == 0 ||
       options.limits.relay.max_duration.count() <= 0 || options.limits.relay.reservation_ttl.count() <= 0) {
      throw_p2p_error(error_kind::invalid_options, "invalid P2P node limits");
   }
}

} // namespace fcl::p2p
