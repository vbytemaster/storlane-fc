# fcl_asio

`fcl_asio` owns the shared async runtime primitives used by FCL networking and
applications. It wraps Boost.Asio with explicit runtime ownership, blocking
boundaries and a priority task scheduler.

## When To Use

- A library needs an owned `boost::asio::io_context` runtime.
- Background work needs bounded queues, cancellation and deterministic shutdown.
- Blocking code must be isolated from coroutine-first paths.

## When Not To Use

- Do not use it as a generic global job system.
- Do not encode product priority names here; `fcl::asio::priority` is numeric.
- Do not expose `std::future` as public async API. FCL async APIs use
  `boost::asio::awaitable<T>`.

## Public Modules

- `fcl.asio.runtime` — owned `io_context` and worker threads.
- `fcl.asio.blocking` — explicit blocking boundary helpers.
- `fcl.asio.task_scheduler` — bounded priority scheduler and task handles.

Target: `fcl_asio`.

Dependencies: Boost.Asio and threads.

## Examples

### Own A Runtime

```cpp
import fcl.asio.runtime;

auto runtime = fcl::asio::runtime{{.worker_threads = 2, .thread_name = "worker"}};
auto& io = runtime.context();
runtime.stop();
```

### Bridge Coroutine Code To A Blocking Entrypoint

Use `blocking::run` at the edge of a command-line tool, test, migration or
small utility. Do not push it into reusable async APIs.

```cpp
#include <boost/asio/awaitable.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/this_coro.hpp>
#include <boost/asio/use_awaitable.hpp>

#include <chrono>

import fcl.asio.blocking;
import fcl.asio.runtime;

boost::asio::awaitable<int> calculate_after_delay() {
   auto executor = co_await boost::asio::this_coro::executor;
   auto timer = boost::asio::steady_timer{executor, std::chrono::milliseconds{25}};
   co_await timer.async_wait(boost::asio::use_awaitable);
   co_return 42;
}

auto runtime = fcl::asio::runtime{};
auto value = fcl::asio::blocking::run(runtime, calculate_after_delay());
```

### Bound A Blocking Wait With Timeout

`run_for` is useful for tests and operator probes where a missing callback should
be reported as a timeout, not as an infinite hang.

```cpp
import fcl.asio.blocking;

auto completed = fcl::asio::blocking::run_for(
   runtime,
   wait_for_readiness_probe(),
   std::chrono::seconds{5});

if (!completed) {
   report_unavailable("readiness probe timed out");
}
```

### Submit Priority Work

```cpp
import fcl.asio.task_scheduler;

auto scheduler = fcl::asio::task_scheduler{runtime, {.max_active_tasks = 4}};
auto handle = scheduler.submit({
   .priority = fcl::asio::priority{100},
   .name = "metadata-refresh",
   .work = [] { refresh_metadata(); },
});

co_await handle.wait();
```

### Use Numeric Priorities Without Product Vocabulary

The scheduler only knows numbers. A product can define its own named constants
near the component that owns those meanings.

```cpp
namespace priorities {
   inline constexpr auto foreground = fcl::asio::priority{500};
   inline constexpr auto background = fcl::asio::priority{-100};
}

auto hot = scheduler.submit({
   .priority = priorities::foreground,
   .name = "foreground-read",
   .work = [] { serve_foreground_request(); },
});

auto cold = scheduler.submit({
   .priority = priorities::background,
   .name = "background-refresh",
   .work = [] { refresh_cache_index(); },
});
```

### Cancel Delayed Work

```cpp
auto handle = scheduler.submit_after(
   {.priority = fcl::asio::priority{0}, .name = "retry", .work = [] { retry(); }},
   std::chrono::milliseconds{250});

handle.cancel();
```

### Shut Down Deterministically

Call `stop()` when the owning service is stopping. Pending work is canceled,
running work is allowed to finish through its normal function body, and handles
can still be awaited by tests.

```cpp
auto pending = scheduler.submit_after(
   {.priority = fcl::asio::priority{0}, .name = "slow-retry", .work = [] { retry(); }},
   std::chrono::minutes{1});

scheduler.stop();
co_await pending.wait();

auto metrics = scheduler.metrics();
assert(metrics.stopped);
assert(metrics.canceled >= 1);
```

## Backpressure And Shutdown

`task_scheduler_options::max_pending_tasks` is a correctness knob. Saturated
queues reject work instead of growing without bound. `stop()` cancels pending
work and lets tests verify deterministic shutdown behavior.

## Typical Mistakes

- Do not sleep/poll inside runtime loops; use timers and scheduler handles.
- Do not let a task capture references whose lifetime is shorter than the
  scheduler queue.
- Do not hide blocking I/O in coroutine paths without the blocking boundary.

## Tests

`test_fcl_asio` covers priority/FIFO ordering, delayed execution, cancellation,
queue saturation and shutdown cancellation.
