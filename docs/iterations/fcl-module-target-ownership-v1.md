# FCL Module Target Ownership v1

## Кратко

Эта итерация убирает временный общий `fcl_modules` bridge. Теперь каждый доменный target сам владеет своими публичными `.cppm` через `FILE_SET CXX_MODULES`.

`BMI` — Binary Module Interface, служебный файл компилятора для C++ modules. Он нужен сборке, но не является архитектурным слоем.

## Целевое Правило

Каждая библиотека FCL объявляет свои module interface files внутри собственного `CMakeLists.txt`:

```cmake
add_library(fcl_raw OBJECT ...)
fcl_target_modules(fcl_raw raw)
```

Helper `fcl_target_modules(target lib)` подключает только:

```text
libraries/<lib>/include/fcl/<lib>/*.cppm
```

через:

```cmake
FILE_SET CXX_MODULES
BASE_DIRS libraries/<lib>/include
```

Общий target, который владеет всеми `.cppm`, запрещён.

## Что Было Исправлено

- Удалён общий сборщик module files.
- Удалён target `fcl_modules`.
- Убраны `$<TARGET_OBJECTS:fcl_modules>` и `target_link_libraries(... fcl_modules ...)`.
- Доменные targets остались `OBJECT`, но теперь сами объявляют свои `.cppm`.
- `fcl` остался только агрегатором object targets and public link dependencies.
- `fcl_tui` получает свои module files только если TUI target реально создан.

## Разорванные Циклы

Удаление общего bridge показало реальные циклы, которые раньше маскировались:

- `variant -> exception -> log -> variant`;
- old reflection-to-variant conversion bridge pulled variant concerns into reflection;
- `json -> log -> variant/json`;
- `websocket -> http -> websocket`;
- `crypto -> log/exception -> variant/raw/reflect`.

Принятые решения:

- Нижние value/reflection modules бросают standard exceptions там, где старая FC-иерархия исключений создавала обратную зависимость.
- `variant::format_string` больше не импортирует `fcl_json`; object/array rendering для format-string compatibility живёт локально в `variant`.
- `websocket` больше не импортирует `http` public DTO; generic endpoint adapter keeps source compatibility for callers with `http::base_url`.
- Crypto не импортирует logger из горячих/низких модулей.

## Запрещено

- Возвращать `fcl_modules`, `FCL_MODULE_FILES` или общий BMI-pool.
- Линковать домены через umbrella target ради module ordering.
- Прятать import cycle за aggregator target.
- Делать `fcl` владельцем public module interfaces.

Если появляется новый цикл, надо:

- перенести shared value type в нижний домен;
- выделить отдельный small target;
- убрать обратный import;
- или заменить зависимость на нейтральный standard type.

## Проверки

Обязательные checks:

```sh
rg "fcl_modules|FCL_MODULE_FILES|fcl_collect_module_files" CMakeLists.txt libraries tests AGENTS.md
rg "target_link_libraries\\([^\\)]*fcl_modules|\\$<TARGET_OBJECTS:fcl_modules>" CMakeLists.txt libraries
find libraries -path '*/include/fcl/*/*' -type d -print
git diff --check
```

Build/test baseline:

```sh
cmake --build build/fcl-module-targets-debug -j 1 \
  --target fcl test_fcl test_fcl_asio test_fcl_app test_fcl_schema test_fcl_config \
  test_fcl_yaml test_fcl_program_options test_fcl_http_websocket test_fcl_quic_p2p test_fcl_tui

ctest --test-dir build/fcl-module-targets-debug \
  --output-on-failure \
  -R "^(test_fcl|test_fcl_asio|test_fcl_app|test_fcl_schema|test_fcl_config|test_fcl_yaml|test_fcl_program_options|test_fcl_http_websocket|test_fcl_quic_p2p|test_fcl_tui)$" \
  --timeout 300
```
