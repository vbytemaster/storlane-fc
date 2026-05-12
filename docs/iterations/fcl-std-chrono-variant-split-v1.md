# FCL Std Chrono + Variant Split v1

## Summary

This pass removes the old FC-style time source API and moves public time values to `std::chrono`.

Compatibility remains binary, not source-level: `fcl::raw::pack` and `fcl::raw::unpack` keep the old FC byte layout for retained time semantics.

## Chrono Boundary

`fcl_core` owns only neutral chrono helpers:

- ISO formatting and parsing for `std::chrono::sys_time<std::chrono::microseconds>`;
- ISO formatting and parsing for `std::chrono::sys_seconds`;
- old FC wire conversion helpers for microseconds and seconds.

`fcl_core` does not import `variant`, `raw`, `json` or `log`.

## Raw Compatibility

The old FC wire layout is preserved:

- `std::chrono::sys_time<std::chrono::microseconds>` packs as `uint64` microseconds since epoch;
- `std::chrono::sys_seconds` packs as `uint32` seconds since epoch;
- `std::chrono::microseconds` packs as the old bit-compatible `uint64` duration representation.

Values outside the old seconds range are rejected with `std::out_of_range`.

## Variant Boundary

`variant` remains its own library. Chrono conversions to and from ISO strings live in `fcl_variant`, not `fcl_core`.

The old monolithic variant module was split into smaller public modules. `variant_object` remains in the same owning `value` module as `variant`, because C++ named modules do not allow a class to be forward-declared in one module and defined in another. The attempted split is documented as rejected module ownership, not as a hidden bridge.

## Dead Module Cleanup

Empty or compatibility-only modules are forbidden. This pass removes the old empty vector module and the two raw compatibility-facade modules.

Library aggregate modules may contain only `export import` statements. Non-aggregate modules must expose real public API.

## Acceptance

Required checks:

```sh
rg "<old-time-module-or-old-fcl-time-type-patterns>" libraries tests docs AGENTS.md
rg "<removed-empty-vector-or-raw-facade-module-patterns>" libraries tests docs AGENTS.md
wc -l libraries/variant/include/fcl/variant/*.cppm
```

All removed-source checks must be empty, and no single variant public module may exceed 350 lines.
