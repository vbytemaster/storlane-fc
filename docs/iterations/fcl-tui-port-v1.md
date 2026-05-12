# FCL TUI Port v1

## Summary

This pass ports the neutral terminal UI primitives from Storlane into FCL as
`fcl_tui`. Storlane remains a donor/reference only; no downstream product
screens, authority flows, or application vocabulary move into FCL.

## Donor Trace

| Donor | Accepted pattern | Rejected pattern | FCL target | Proof |
| --- | --- | --- | --- | --- |
| Notcurses core | Terminal initialization, input polling, capability detection and render lifecycle stay behind one backend boundary. | Exposing Notcurses structs or headers through public modules. | `fcl::tui::screen_runner` and private `runner.cpp` backend. | Public include grep for `notcurses`; `test_fcl_tui`. |
| FTXUI component model | Value models and deterministic render helpers are testable without a terminal. | Pulling FTXUI or copying its event loop. | `status_badge_model`, tables, forms, modals, action bars and shell render helpers. | Render model tests. |
| k9s operator UX | Navigation is explicit UI state over resource-like views. | Kubernetes-specific resource model. | `navigation_model` and `navigation_stack`. | Push/pop/select tests. |
| Ceph health/status | Degraded reasons are operator-visible, not hidden in backend state. | Dashboard/UI as source of truth. | `terminal_capabilities::degraded_reason`. | Capability detection no-throw test. |
| Syncthing diagnostics UI | Event feeds are diagnostic visibility only. | UI events as business-flow coupling. | `event_log_model` redacted render output. | Event log redaction test. |

## Implementation Rules

- Public API lives in flat `libraries/tui/include/fcl/tui/*.cppm` module files.
- Implementation `.cpp` files live directly in `libraries/tui`.
- `fcl_tui` depends on Notcurses only as a private backend dependency.
- `FCL_ENABLE_TUI=ON` attempts to discover `notcurses-core`; if unavailable,
  `fcl_tui` and `test_fcl_tui` are skipped without breaking the rest of FCL.
- UI is not a security boundary. Redaction is a display safety layer only.

## Validation

- Build `fcl` and `test_fcl_tui`.
- Run `test_fcl_tui`.
- Check that no `storlane::` or `storlane.` names remain in `libraries/tui`
  or `tests/tui`.
- Check that Notcurses headers do not appear in public TUI modules or tests.
