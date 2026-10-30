# SV-B2-004 — birth workspace evidence

2026-10-29. Linux evidence для B2-004; весь B2 этим документом не принимается.

## Reference и production seam

Frozen source: `lib/xtra/sv/login/reference/`, final variant D. One-off replay
[capture-reference.cjs](evidence/sv-b2-004/capture-reference.cjs) проходит browser
controls последовательно, без подстановки birth state. Background 0,
reduced motion, DPR 1, Liberation Mono; supported sizes 1440×900 и 1024×768.
Для видимого trait выбраны Mage/Maia (prototype отображает class как Istar).
Captured families: sex, class, race, trait, body, stats, unused-points warning,
mode, Guide, quit. Prototype числа и rules не используются native model.
Computed styles сохранены рядом с PNG.

Native one-off [capture-native.py](evidence/sv-b2-004/capture-native.py) основан
на существующем B2-003 replay: настоящий executable, localhost contact/setup/login
packets, XTest input и PNG actual X11 window. Production session/interaction
определяет choices и transitions. Software/OpenGL используются в Linux.

## Проверки

- `make -C src -f makefile.sv tomenet-sv -j4` — PASS, warnings-as-errors.
- Согласованный subset `sv_name_editor_submit_checks.py` — PASS, ASan/UBSan,
  оба существующих build orders. Сценарии: navigate, mouse, alternate-mouse,
  long class/race/trait viewport, ordered/point stats mouse, unspent warning,
  mode-back, random, quit. Исходный name prelude и exact submitted bytes сохраняются.
- `SV_GUIDE_CASES=point SV_GUIDE_ORDERS=1` existing Guide check — PASS:
  exact placeholder, actual caller return, edited point draft, authoritative ACK,
  MOTD и live handoff.
- `SV_SUBMISSION_CASE=waiting,partial,ack,retry` existing submission check — PASS:
  16266 exact final handshake bytes, keepalive, once-only request и generation.
- После локальных исправлений повторены только affected existing warning/mouse,
  mode-back/mouse и three long-grid cases — PASS. Assertions семантики не ослаблялись.

Все команды — [commands.txt](evidence/sv-b2-004/commands.txt), stdout — рядом.
Новые suites/harness/version matrix не добавлялись. Existing mouse fixtures
переведены на production rectangles и wheel hit regions; offset clicks/scroll
изменены для final reference grid. Keep editing проверяется в существующем
warning case: открыть → click cancel → снова открыть, сохраняя прежний итоговый
pending assertion. Existing point mouse подтверждает через настоящий modal button.
Windows/MinGW/Wine deferred согласно текущему правилу платформ.

## Native visual evidence

Final executable SHA-256:
`93bc52c20a5ae3dc0dab240a8c6cb37ae9c5ea63072e0afdb42923187407cb87`.
Один и тот же final executable используется для retained native captures;
temporary executable находится в `/tmp/`, не сохраняется в Git.
Peer передаёт Human/Elf/Dwarf, Warrior/Ranger/Mage и Dragon/Phoenix настоящим
setup packet; выбираются Mage/Dwarf/Phoenix/Fruit bat. Adjustments/recommendations
в этом controlled setup нулевые; prototype Istar/Maia цифры не подставляются.
Name request `0d0c4361726f6c00` включает keepalive `0d`, login `0c` и `Carol`.
Во всех family captures используется реальный birth interaction после wire
permission, не preview scene. Point и ordered stats используют предусмотренный
native setup flag; оба orders дополнительно проверяет existing core suite.

| Family | Reference | Software | OpenGL |
| --- | --- | --- | --- |
| sex | [1440×900](evidence/sv-b2-004/reference-sex-1440x900.png) / [1024×768](evidence/sv-b2-004/reference-sex-1024x768.png) | [1440×900](evidence/sv-b2-004/software-sex-1440x900.png) / [1024×768](evidence/sv-b2-004/software-sex-1024x768.png) | [1440×900](evidence/sv-b2-004/opengl-sex-1440x900.png) / [1024×768](evidence/sv-b2-004/opengl-sex-1024x768.png) |
| class | [1440×900](evidence/sv-b2-004/reference-class-1440x900.png) / [1024×768](evidence/sv-b2-004/reference-class-1024x768.png) | [1440×900](evidence/sv-b2-004/software-class-1440x900.png) / [1024×768](evidence/sv-b2-004/software-class-1024x768.png) | [1440×900](evidence/sv-b2-004/opengl-class-1440x900.png) / [1024×768](evidence/sv-b2-004/opengl-class-1024x768.png) |
| race | [1440×900](evidence/sv-b2-004/reference-race-1440x900.png) / [1024×768](evidence/sv-b2-004/reference-race-1024x768.png) | [1440×900](evidence/sv-b2-004/software-race-1440x900.png) / [1024×768](evidence/sv-b2-004/software-race-1024x768.png) | [1440×900](evidence/sv-b2-004/opengl-race-1440x900.png) / [1024×768](evidence/sv-b2-004/opengl-race-1024x768.png) |
| trait | [1440×900](evidence/sv-b2-004/reference-trait-1440x900.png) / [1024×768](evidence/sv-b2-004/reference-trait-1024x768.png) | [1440×900](evidence/sv-b2-004/software-trait-1440x900.png) / [1024×768](evidence/sv-b2-004/software-trait-1024x768.png) | [1440×900](evidence/sv-b2-004/opengl-trait-1440x900.png) / [1024×768](evidence/sv-b2-004/opengl-trait-1024x768.png) |
| body | [1440×900](evidence/sv-b2-004/reference-body-1440x900.png) / [1024×768](evidence/sv-b2-004/reference-body-1024x768.png) | [1440×900](evidence/sv-b2-004/software-body-1440x900.png) / [1024×768](evidence/sv-b2-004/software-body-1024x768.png) | [1440×900](evidence/sv-b2-004/opengl-body-1440x900.png) / [1024×768](evidence/sv-b2-004/opengl-body-1024x768.png) |
| stats | [1440×900](evidence/sv-b2-004/reference-stats-1440x900.png) / [1024×768](evidence/sv-b2-004/reference-stats-1024x768.png) | [1440×900](evidence/sv-b2-004/software-stats-point-1440x900.png) / [1024×768](evidence/sv-b2-004/software-stats-point-1024x768.png) | [1440×900](evidence/sv-b2-004/opengl-stats-point-1440x900.png) / [1024×768](evidence/sv-b2-004/opengl-stats-point-1024x768.png) |
| warning | [1440×900](evidence/sv-b2-004/reference-warning-1440x900.png) / [1024×768](evidence/sv-b2-004/reference-warning-1024x768.png) | [1440×900](evidence/sv-b2-004/software-warning-1440x900.png) / [1024×768](evidence/sv-b2-004/software-warning-1024x768.png) | [1440×900](evidence/sv-b2-004/opengl-warning-1440x900.png) / [1024×768](evidence/sv-b2-004/opengl-warning-1024x768.png) |
| mode | [1440×900](evidence/sv-b2-004/reference-mode-1440x900.png) / [1024×768](evidence/sv-b2-004/reference-mode-1024x768.png) | [1440×900](evidence/sv-b2-004/software-mode-1440x900.png) / [1024×768](evidence/sv-b2-004/software-mode-1024x768.png) | [1440×900](evidence/sv-b2-004/opengl-mode-1440x900.png) / [1024×768](evidence/sv-b2-004/opengl-mode-1024x768.png) |
| guide | [1440×900](evidence/sv-b2-004/reference-guide-1440x900.png) / [1024×768](evidence/sv-b2-004/reference-guide-1024x768.png) | [1440×900](evidence/sv-b2-004/software-guide-1440x900.png) / [1024×768](evidence/sv-b2-004/software-guide-1024x768.png) | [1440×900](evidence/sv-b2-004/opengl-guide-1440x900.png) / [1024×768](evidence/sv-b2-004/opengl-guide-1024x768.png) |

Дополнительно сохранены ordered stats, edited point stats, Back→stats и
1440×900→1024×768 mode resize. Resize filenames указывают исходное окно;
actual PNG после resize — 1024×768. Native Quit/Back сохраняет B baseline caller:
Q завершает birth и возвращает overview, а не копирует browser navigation.
Reference quit представлен отдельно. Guide содержит exact
`The guide is in development`; полный Guide C062 не реализуется.

Visual adaptations: server data/list cardinality отличаются от browser demo;
нет mock numeric values и step-list. Native stats ordering имеет отдельные
choice controls; point stats показывают доступные server adjustments.
Сохранены black/green/gold family, summary left, choices/detail60/40, selection,
fixed controls и независимый overflow. Native many-traits использует две колонки
wide с scroll, одну minimum; mode всегда две колонки с overflow при native seventh
mode. Summary оформлена проще browser decorative portrait/stat boxes, сохраняя
выбранные значения и scroll; pixel-perfect gate не вводится.
Known inherited raster differences B2-001: background без CSS blur,
Liberation Serif вместо Georgia, square borders/focus raster.

## Диагностика и review

Первые XTest opening keys на KDE/Xwayland иногда оставляли overview без открытия
name; actual mouse Create активирует окно, после чего replay проверяет name
wire request перед capture. Старые incoherent frames удалены из retained evidence;
диагностика находится в `/tmp/b2-004-native-investigation/` и
`/tmp/b2-004-freeze1-diagnostic/`. Один startup timeout закончился локальным
завершением только собственного процесса; provider/IME матрица не вводилась.
Child executable использует session-only credential provider (unavailable DBUS);
compositor layout временно Latin и восстанавливается после replay.

`SV endpoint startup failed: Parameter 'texture' is invalid` после local termination
не доказывает render defect: baseline text cache может оставить SDL error при
`SDL_DestroyTexture(NULL)`. Успешные retained scenes подтверждаются actual window
PNG и живым protocol peer до termination; server gameplay/handoff этими
screenshot replay не заявляется. ACK/handoff подтверждает existing production suite.

Standards FREEZE2 — PASS, 0 mandatory findings. Spec source review после
исправлений descriptions/hotkeys, Keep editing и modal Enter — 0 findings;
visual review сверил основные семьи, затем исправлены grid и compact geometry.
Неблокирующее замечание duplicated control mapping записано отдельно в
`docs/sv-improvements.md`. Final Spec source+visual review FREEZE4a — PASS, 0 findings; проверены
corrected compact/race/trait/mode grids и OpenGL spotcheck class/race/point/warning.

Graph обновлён: 17918 nodes / 46801 edges; known partial AST extraction77 и
no-symbol155 сохраняются, CLI не заявляет semantic refresh документов.
Scoped `git diff --check` для evidence и existing test adaptation — PASS.

Selected visibility: Down×5 выбирает Charisma, scroll показывает rows2–6;
после1440×900→1024×768 selected Charisma остаётся видна, footer доступен.
[Software](evidence/sv-b2-004/scroll/software-stats-resize-selected-last-1440x900.png),
[OpenGL](evidence/sv-b2-004/scroll/opengl-stats-resize-selected-last-1440x900.png).
Replay ещё2 runs тем же final executable; native-capture.json находится отдельно
в `scroll/`, чтобы не заменить основной8-run transcript.
Retained artifacts и final production source inputs зафиксированы в
[SHA256.json](evidence/sv-b2-004/SHA256.json).
