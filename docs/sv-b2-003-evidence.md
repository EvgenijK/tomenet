# SV-B2-003 — character overview и name evidence

2026-10-29. Linux evidence для B2-003; приёмка всего B2 не заявляется.

## Production и reference

Frozen reference: bundled `lib/xtra/sv/login/reference/`, variant D,
background 0, reduced motion, DPR 1, Liberation Mono. Reference overview/name/edit/error
сняты в 1440×900 и 1024×768. В browser остаётся prototype roster; native executable
получает 10 Hero0…Hero9 (для overview/name) и 12 Hero0…Hero11 (для overflow/resize) через controlled localhost peer настоящими contact/setup/login
пакетами, используя wire sequence существующего `sv_login_live_checks.py`.
Races/classes в peer отсутствуют; native честно показывает unknown/pending данные.
Это вход production protocol, не отдельная реализация клиента для тестов.

Final executable SHA-256:
`e3ac1e4d76a3ec45a79c143be75c36a62e07c6b2d7e9d576fc37f09b88d6688c`.
Capture запускается из отдельной копии binary, чтобы не блокировать `make`.

Артефакты сохранены в [docs/evidence/sv-b2-003/](evidence/sv-b2-003/):
[команды](evidence/sv-b2-003/commands.txt), [build/dependencies](evidence/sv-b2-003/build.txt),
[capture script](evidence/sv-b2-003/capture-native.py),
[browser script](evidence/sv-b2-003/capture-reference.cjs),
[SHA-256 manifest](evidence/sv-b2-003/SHA256.json).
Это one-off evidence replay, не новый test harness и не новая suite.
История диагностики перенесена в `/tmp/sv-b2-003-investigation/`; временная копия executable удалена, binary в Git не сохраняется.

## Linux проверки

- `make -C src -f makefile.sv tomenet-sv -j4` — PASS, warnings-as-errors.
- `python3 tests/sv_login_checks.py` — PASS, existing core/parser/interaction/native adapter.
- `python3 tests/sv_login_live_checks.py` — PASS, executable contact/login socket path.
- `python3 tests/sv_name_rejection_recovery_checks.py` — PASS, ASan/UBSan,
  existing rejection/retry/cancel и exclusive recovery scenarios.
- `SV_BIRTH_CASE=navigate,mouse,random,quit,alternate-mouse python3 tests/sv_name_editor_submit_checks.py`
  — PASS, existing minimal subset, ASan/UBSan, оба production choice orders.
  Общий name prelude проверяет exact submitted `Carol$`, byte-limit/truncation,
  UTF-8 input, keyboard edit, mouse selection и clipboard paste.

Новые suites не добавлялись. Windows/MinGW/Wine deferred по текущему правилу платформ.

## Runtime observations и ограничения

Production input replay использует XTest. Реальный KDE/Wayland text layout
управляется compositor `org.kde.KeyboardLayouts`; один `XkbLockGroup` менял
Xwayland state, но actual SDL TEXT_INPUT продолжал выдавать Russian
`т у ц р у к щ` вместо physical Newhero. Это подтверждено временной диагностикой
public name input и `getLayoutsList/getLayout`: us index0, ru index1.
Replay временно выбирает compositor Latin layout для ASCII, Russian для
unencodable input и восстанавливает исходный layout при выходе.

Первые visual inspections показывали неполные scene; render/present failure
в diagnostic replay не подтвердился. При дальнейшей проверке два отдельных файла
с одинаковыми PNG и decoded-pixel SHA-256 отображались инструментом просмотра
как будто с разным содержимым. Поэтому конкретная причина первоначального
наблюдения не доказана; исправление client defect не заявляется. Accepted evidence
использует coherent actual X11 window PNG, отдельные стабильные имена и hashes.
SDL hook captures и промежуточные диагностические screenshots в retained evidence
не включены. Процесс после capture завершается локально; сообщение
`SV endpoint startup failed:` в replay logs возникает после termination,
не трактуется как успешный server handoff или доказанный rendering defect.

Initial live suite build failed из-за `cp: Text file busy`, когда первый replay
исполнял binary прямо из `src/`; отдельная executable copy устранила конфликт,
повторный build/live suite прошли. Пробный clipboard owner не использован для
final visual evidence; provider/IME matrix, real-server gameplay и physical display
timing этой задачей не подтверждаются.

## Visual review

| State | Reference 1440×900 / 1024×768 | Software 1440×900 / 1024×768 | OpenGL 1440×900 / 1024×768 |
| --- | --- | --- | --- |
| overview | [1440x900](evidence/sv-b2-003/reference-overview-1440x900.png) / [1024x768](evidence/sv-b2-003/reference-overview-1024x768.png) | [1440x900](evidence/sv-b2-003/software-overview-1440x900.png) / [1024x768](evidence/sv-b2-003/software-overview-1024x768.png) | [1440x900](evidence/sv-b2-003/opengl-overview-1440x900.png) / [1024x768](evidence/sv-b2-003/opengl-overview-1024x768.png) |
| name | [1440x900](evidence/sv-b2-003/reference-name-1440x900.png) / [1024x768](evidence/sv-b2-003/reference-name-1024x768.png) | [1440x900](evidence/sv-b2-003/software-name-1440x900.png) / [1024x768](evidence/sv-b2-003/software-name-1024x768.png) | [1440x900](evidence/sv-b2-003/opengl-name-1440x900.png) / [1024x768](evidence/sv-b2-003/opengl-name-1024x768.png) |
| edit | [1440x900](evidence/sv-b2-003/reference-edit-1440x900.png) / [1024x768](evidence/sv-b2-003/reference-edit-1024x768.png) | [1440x900](evidence/sv-b2-003/software-edit-1440x900.png) / [1024x768](evidence/sv-b2-003/software-edit-1024x768.png) | [1440x900](evidence/sv-b2-003/opengl-edit-1440x900.png) / [1024x768](evidence/sv-b2-003/opengl-edit-1024x768.png) |
| error | [1440x900](evidence/sv-b2-003/reference-error-1440x900.png) / [1024x768](evidence/sv-b2-003/reference-error-1024x768.png) | [1440x900](evidence/sv-b2-003/software-error-1440x900.png) / [1024x768](evidence/sv-b2-003/software-error-1024x768.png) | [1440x900](evidence/sv-b2-003/opengl-error-1440x900.png) / [1024x768](evidence/sv-b2-003/opengl-error-1024x768.png) |

Edit сохраняет actual draft `Newhero`, double click выделяет его; error сохраняет
тот же draft и показывает actual encoding error после Russian text input.
Перенесены compact inline rows, diamond level, mode/location, slot placeholders,
fixed footer и inner scrolling; отдельная overview detail panel отсутствует.
Name использует full-width creation family, summary и field/caret/selection.
Native adaptations: unavailable race/class и ещё не выбранные birth значения;
нет mock stats; reorder не реализован; доступны existing semantic actions.
Web affordances reorder S/I/A намеренно исключены из overview; их owner — C039.
Составные и неизвестные mode flags сохраняются как numeric `Mode 0xNNNN`,
вместо потери части server bits в единственной подписи. Native Back/quit,
ordinary/exclusive availability и feedback используют существующую B-семантику.
Known raster differences: B2-001 background shading без CSS blur, Liberation Serif
вместо browser Georgia, square border/focus raster, нативные надписи/status и
bounded unavailable summary. Pixel-perfect gate не вводился.

Реальный overflow: 12 wire rows, после Down×11 в minimum viewport остаются
Hero2…Hero11, selected Hero11 виден. [Software scroll](evidence/sv-b2-003/software-scroll-1024x768.png),
[OpenGL scroll](evidence/sv-b2-003/opengl-scroll-1024x768.png).
После resize 1440×900→1024×768 selected Hero11 сохраняется:
[software](evidence/sv-b2-003/software-resize-1440x900.png),
[OpenGL](evidence/sv-b2-003/opengl-resize-1440x900.png).
Software resize и direct-minimum scroll имеют одинаковые PNG/pixel hashes;
фактический output resized frame — 1024×768, исходный size в имени фиксирует
начальное окно. Keyboard/mouse slot outcome дополнительно проверяется
existing production adapter core assertions; новая behavioral модель не создавалась.

Независимые final Standards и Spec review завершены после корректировок; открытых замечаний — 0. Final Spec review сопоставил source и сохранённые native/reference frames.

## Final consistency

Scoped `git diff --check` прошёл. Source graph обновлён командой
`.graphify-venv/bin/graphify update .`: 17864 nodes, 46619 edges. Как и до задачи,
extractor сообщает partial syntax extraction в 77 файлах и отсутствие symbols
в 154 файлах; semantic extraction новых документов/картинок не заявляется.
Source review и production checks остаются основанием результата.
