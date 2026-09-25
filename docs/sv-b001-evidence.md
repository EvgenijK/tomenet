# SV-B-001 — implementation evidence

Дата: 2026-09-24. База реализации: `791528ff544c14d80b3d7f7df3c3bc02b341bf34`.
Статус: implementation-ready для следующих B-задач на Linux; полное acceptance остаётся pending.

Production-путь: `src/tomenet-sv --endpoint` показывает native whole-window список,
читает metaserver TCP/8801 в фоновом SV provider, использует порядок configured →
`meta.tomenet.eu` → `37.187.75.24`, обновляет ping без потери выбора и передаёт
выбранные host spelling, port и protocol как `SvEndpointChoice`. Ручной адрес,
`--server` и `--port` используют тот же endpoint model. Escape завершает startup
без попытки соединиться с игровым сервером. Игровое соединение принадлежит SV-B-002.
SDL input идёт через production physical-byte router; text/clipboard — через
bounded byte editor. Linux/Windows SDL macro trigger bytes, sticky modifiers,
prompt navigation, confirm contract и raw-key serializer проверены на своих
ранних caller seams. Полнота caller integration остаётся за B-006, B-029,
B-030 и B-043.

Конфигурация проверок: Linux amd64, clang 22.1.8, SDL3 3.4.16,
SDL3_ttf 3.2.2, FreeType 26.6.20, `SDL_VIDEODRIVER=dummy`,
`SDL_RENDER_DRIVER=software`, изолированный временный profile root и `lib/`.
Build key: `linux-9dd5a1e98a672a5b25cd` (`src/.sv-build/linux/.../build.txt`).
Источник ресурса `lib/xtra/font/CascadiaMono-Regular.ttf`:
SHA-256 `06520d032ec274fa5040b22c6f4a1d829081b24ba40b2da56dae89bf10c7b481`.
Проверка исходников и fixture: `metaserver.c` SHA-256
`620a4e701c9ce2b8320b3a2666796756148f3236cd6135a16514a2cee43d3bcc`,
`physical.c` `63b4653493fed38a9edae5cae9e21e5172c7aff9b34f270bb182c9e04990e9d1`,
`tests/sv/metaserver-live.c` `77356293ecb79654c71084a093d408230a6cbc2e4b97b0915c075a5d02b28db7`.

Автоматизированные результаты:

- `tests/sv_endpoint_checks.py`: pass. Production editor, SDL adapter normal/sticky,
  physical key bytes, native scene row selection with pending ping, Escape,
  clipboard error/limits, metaserver failure→manual.
- `tests/sv_metaserver_live_checks.py`: pass вне sandbox на loopback. Production
  TCP provider получает split XML, parser сохраняет protocol, ping process fixture
  передаёт 42 ms, выбранный endpoint остаётся `localhost:18350`.
  Отмена во время активного ping возвращается быстрее 200 ms.
- `tests/sv_request_checks.py`: pass, 372 split/HP/cancel/abort cases и
  SDL physical trigger → SV router → prompt reply/raw-key wire bytes.
- `tests/sv_shell_smoke.py`, `sv_message_checks.py`, `sv_hp_checks.py`,
  `sv_lifecycle_checks.py`, `sv_arch_checks.py`, `sv_runtime_checks.py`,
  `sv_runtime_producer_checks.py`, `sv_review_checks.py`, `sv_html_checks.py`,
  `sv_evidence_checks.py`, `sv_reconciliation_checks.py`: pass.

Открытые gates: `sv_checkpoint_checks.py` и `sv_capabilities_checks.py` ожидают
закреплённый Stage A digest `src/makefile.sv`; добавление B-модулей делает
этот источник stale (`source-digest`), поэтому positive evidence fixture больше
не проходит. Canonical registry здесь не менялся; Stage A evidence/fixture надо
пересобрать при сводной проверке, сохранив исходные baseline anchors. MinGW
i686 build пока заблокирован отсутствием SDL3/SDL3_ttf/FreeType development
dependencies в установленном cross toolchain. Actual Windows 10/11 и
accelerated renderer, а также late caller integration checks остаются pending.
