# FCL Dependency Hygiene + Reflect/Variant Boundary v1

## Summary

This pass fixes two boundary leaks:

- described type conversion to and from `fcl::variant` belongs to `fcl_variant`;
- external dependencies belong to the target that actually owns the public include or implementation usage.

`fcl_reflect` remains a small Boost.Describe metadata layer, not a variant adapter.

## Boundaries

- `fcl_core` owns neutral type-name helpers only.
- `fcl_reflect` owns described-object and described-enum metadata helpers.
- `fcl_variant` owns `to_variant` and `from_variant` for described enums and objects through `fcl.variant.described`.
- `fcl_reflect` must not link `fcl_variant`.

`boost::dynamic_bitset<std::uint8_t>` stays as the `fcl::dynamic_bitset` implementation in this pass because the standard library has no runtime-size bitset with the block API required by the retained raw layout.

## Dependency Ownership

The umbrella `fcl` target aggregates FCL domain targets. It must not carry broad Boost link entries as a shortcut for leaf targets.

Examples of accepted ownership:

- program-options backend owns `Boost.Program_options`;
- HTTP owns Boost.URL and the required Asio/Beast public surface;
- JSON owns Boost.Iostreams in its backend implementation;
- logging owns Boost.DLL where it resolves runtime symbol information;
- variant and raw own Boost headers needed by dynamic bitsets, multiprecision and multi-index conversions.

If a hidden dependency appears during build, add it to the concrete owner target instead of adding it to the umbrella target.

## Checks

Required checks:

```sh
rg "<old-reflection-variant-bridge-module-or-path>" libraries tests docs AGENTS.md
rg "fcl_reflect PUBLIC[\\s\\S]*fcl_variant|import fcl\\.variant" libraries/reflect CMakeLists.txt
rg "Boost::(url|date_time|chrono|iostreams|interprocess|multi_index|dll|multiprecision|program_options|thread)" CMakeLists.txt
rg "get_typename" libraries/reflect
git diff --check
```
