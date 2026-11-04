# SV-B2-005 — MOTD C и общие native states

2026-10-29. Статус: implemented. Linux build, существующие core проверки,
software/OpenGL visual replay и два независимых code review завершены.
Открытых замечаний нет. Полная приёмка B2 остаётся за B2-006.

## Production implementation

MOTD использует `ui/motd.h` → `sv_login_shell_motd`, вызываемый
`sv_endpoint_render` из действительного endpoint loop. Источник — read-only
`SvLoginView` текущей pregame session. Decoder ограничен 23 строками по120 bytes,
останавливает отдельную строку по NUL, сохраняет пустые строки, пробелы, исходные
column positions и glyph bytes. Это presentation decoder форматирования setup
field; protocol/contact и session ownership не менялись.

Распознаются все62 color letters из baseline `color_char_to_attr`, удвоенный
FF→`{`, восстановление first/previous colors `FF-`/`FF.`, neutral `FF%`.
Unknown marker оставляет его символ видимым в текущем цвете. Ширина определяется
максимальным decoded visible extent, а не120-byte stride. Пустой MOTD очищает
whole-window сцену до чёрного без client headings, shell D и overlays.

Вписывается полная23-row композиция с24 logical units отступа. Final TTF metrics
подбираются одновременно по ширине и высоте, поддерживают увеличение/уменьшение;
целочисленный raster центрируется, рисуется1:1, без steady texture scaling,
wrap, scroll или crop. Используется bundled Liberation Mono; base RGB — canonical
`src/client/variable.c:client_color_map`, а не shell gold/green palette или
демонстрационные RGB browser prototype. Integer font metrics могут оставлять
немного больший отступ у ограничивающей оси.

`ui/motd-colors.h` сохраняет extended roles и адаптирует baseline
`flick_colour`/`anim2static` без подключения legacy globals: те же цветовые
наборы/распределения,100ms ticks и selector phase. Random sample зависит от
cell/tick; совпадение случайного кадра с legacy не заявляется. Weather, lamp fuel
и map searchlight coordinates отсутствуют в pregame; lamp использует baseline
no-wind distribution, source-light — baseline non-map fire branch. Existing
reduced-motion switch использует static baseline roles; selectors следуют
baseline anim2static phase rules. Настройки gameplay этим ticket не добавляются.

Glyph adaptation: ASCII остаётся исходным; `FONT_MAP_SOLID_X11` byte0x02
отрисовывается полным цветным cell block, byte0x01 — diamond. Высокие печатные
bytes0xA0…0xFF имеют numeric Latin-1 соответствие; неиспользуемые control bytes
показывают один `?` в исходной колонке. Это явный native presentation profile,
не расширение outgoing server encoding и не универсальный legacy bitmap font.

MOTD raster cache содержит собственную копию decoded content и ключи output
size/DPI/animation tick; static content не rasterise каждый кадр. Resize,
renderer reset и teardown пересоздают/освобождают renderer-owned texture.
Error priority перед MOTD сохраняет действительный failure feedback.

## Общие states и caller semantics

Общий `b2_state_frame` предоставляет black/green/gold card, border, heading,
spacing и error color. Его используют contact/auth waiting/failure,
creation-submitted authoritative ACK waiting, unused-points confirmation,
temporary Guide и live-handoff status. Старый plain-text legacy fallback удалён;
неописанные native status states получают тот же card adapter.

- Contact/auth Cancel/Back/Retry использует существующие input handlers и
  прежние availability; после submitted play Retry недоступен.
- Birth Guide содержит exact `The guide is in development`, возвращается
  действительному caller, сохраняет draft; disconnect закрывает child.
- Unused points: Keep editing/Continue остаются actions birth interaction.
- Quit сохраняет прямое завершение текущего caller. Новый confirmation flow
  не вводится там, где его нет в accepted B semantics.
- MOTD any-key, skip и once-per-login остаются в login interaction; displayed
  только после authoritative play ACK, затем live handoff без gameplay actions.

Пароли не передаются новому presentation cache; credential drafts используют
существующий private render-only path. Input/focus/lifecycle ownership сохранён.

## Linux validation

- `make -C src -f makefile.sv tomenet-sv -j4` — PASS, warnings-as-errors.
- `python3 tests/sv_first_session_checks.py` — PASS: success/disconnect/quit/retry,
  ASan/UBSan, production endpoint с controlled local peer.
- `SV_GUIDE_EVIDENCE=/tmp/sv-b2-005-guide python3 tests/sv_guide_motd_checks.py` —
  PASS на final cache/solid correction. Existing
  suite: обе birth order, waiting/partial/ACK/skip/point; independent literal
  final request16266 bytes и authoritative ACK `02 03 7a`.

Existing Guide test дополнен только малым production decoder/color regression и
server setup23×120 с пустыми/colored/unknown/wide final rows. Новых executables,
harness или expanded matrix не добавлено. Initial sandbox socket creation была
запрещена; тот же suite успешно запущен с reviewed escalation для local TCP.
Сохранённые logs: [build](evidence/sv-b2-005/build.log),
[first session](evidence/sv-b2-005/first.log), [Guide/MOTD](evidence/sv-b2-005/guide.log).
Команды: [commands.txt](evidence/sv-b2-005/commands.txt).

Real-server integration, physical provider/IME evidence и gameplay не заявляются.
Windows/MinGW/Wine deferred по текущему правилу платформ.


## Reference/native visual comparison

Approved MOTD C: decision28 / prototype commit
`e271c582cbf89d404deda6189881e531d3775770`, local source
`.scratch/single-window-sdl3-client/motd-prototype.html`.
[Reference replay](evidence/sv-b2-005/capture-reference.cjs) экспортирует весь
logical canvas C, без surrounding prototype controls. Данные short, notice,
unknown, empty берутся из самого prototype. Wide сохраняет119 columns ×23 rows,
но все строки white:119 glyph bytes +2 color marker bytes не помещаются
в120-byte server stride; геометрия variant C не меняется.

[Native replay](evidence/sv-b2-005/capture-native.py) — one-off adaptation
existing B2-004 replay, настоящий `tomenet-sv`, localhost contact/setup/login/play
packets, XTest input и capture actual X11 window. Это visual evidence adapter,
не новая suite или implementation поведения для тестов. Background0,
reduced motion, Liberation Mono, supported logical sizes1024×768/1440×900.
Peer посылает все23 rows настоящим setup field, не подменяет render model.
Final executable SHA-256:
`5955383b5c06d50a4eb9fc63fa3ff5fd06851f687251a4be72346f1dc75405d9`.

| Composition | Reference C | Linux software | Linux OpenGL |
| --- | --- | --- | --- |
| short,1024×768 | [reference](evidence/sv-b2-005/reference-short-1024x768.png) | [native](evidence/sv-b2-005/software-short-1024x768.png) | [native](evidence/sv-b2-005/opengl-short-1024x768.png) |
| wide119,1024×768 | [reference](evidence/sv-b2-005/reference-wide-1024x768.png) | [native](evidence/sv-b2-005/software-wide-1024x768.png) | [native](evidence/sv-b2-005/opengl-wide-1024x768.png) |
| colored notice,1440×900 | [reference](evidence/sv-b2-005/reference-notice-1440x900.png) | [native](evidence/sv-b2-005/software-notice-1440x900.png) | [native](evidence/sv-b2-005/opengl-notice-1440x900.png) |
| unknown composition,1440×900 | [reference](evidence/sv-b2-005/reference-unknown-1440x900.png) | [native](evidence/sv-b2-005/software-unknown-1440x900.png) | [native](evidence/sv-b2-005/opengl-unknown-1440x900.png) |
| empty,1024×768 | [reference](evidence/sv-b2-005/reference-empty-1024x768.png) | [native](evidence/sv-b2-005/software-empty-1024x768.png) | [native](evidence/sv-b2-005/opengl-empty-1024x768.png) |

Все10 final runs завершены. Whole composition центрирована; все23 wide rows и
крайний right bar видимы. Short сохраняет пустые нижние строки, поэтому первые
строки расположены вверху всей23-row композиции, как в C. Resize1440×900→1024×768
пересчитывает fit, сохраняет последнюю строку/column X неизвестного содержимого:
[software](evidence/sv-b2-005/software-unknown-resize-1024x768.png),
[OpenGL](evidence/sv-b2-005/opengl-unknown-resize-1024x768.png).
Integer final metrics дают небольшую разницу полей относительно continuous
browser canvas; canonical server color RGB отличаются от demo palette prototype.
Wrap, crop, scroll, client headings и D overlay в MOTD отсутствуют.

Тот же final binary показывает native adaptations shared states:
[waiting](evidence/sv-b2-005/software-waiting-1440x900.png),
[partial ACK](evidence/sv-b2-005/opengl-partial-1440x900.png),
[Guide](evidence/sv-b2-005/opengl-guide-1440x900.png),
[handoff](evidence/sv-b2-005/software-handoff-1440x900.png).
[Transcript](evidence/sv-b2-005/native-capture.json) фиксирует final request16266
bytes и full ACK `02 03 7a`. Waiting/partial не показывают MOTD, full ACK
разрешает MOTD; actual key A закрывает его и показывает handoff. Skip/once/caller
return дополнительно подтверждены existing production Guide suite.

## Review и завершение

Два независимых агента проверяли code/lifecycle/isolation и spec/visual по снимку
до B2-005, отдельно от предсуществующего dirty tree. Исправлены: extended color
markers, raster под конечный размер вместо постоянного scale, cache неизменной
композиции и special solid glyph0x02. FREEZE2 code review —0 findings;
FREEZE2 spec source+visual review —0 findings. Дополнительных out-of-scope
improvements этим review не выявлено.

`graphify update .` выполнен:17950 nodes /46879 edges. Сохраняются известные
77 partial-AST и155 no-symbol files; semantic refresh документов CLI не заявляет.
Scoped whitespace check выполнен по фактическим изменениям B2-005.
Source inputs и retained evidence фиксирует [SHA256.json](evidence/sv-b2-005/SHA256.json).
