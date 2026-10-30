# B2 — атрибуты, выбор режима и завершение startup stream

Дата: 2026-10-30
Статус: accepted, 2026-10-30, в составе принятого пользователем B2
([решение](../../sv-stage-b2-acceptance.md)).
Labels: bug

Пользователь сообщил неверные цвета предельных атрибутов, фон/рамку выбранной
строки (подтвердил это уточнение), оформление и стрелки mode grid, а после
подтверждения режима — `Session closed: packet decode failed`.
По просьбе пользователя три агента независимо исправляли stats, mode и decoder;
root выполнил общую сборку, минимальные core проверки и renderer captures.

## Исправления

- Атрибуты: frozen reference `value===8/17` относится к BASE, а C ошибочно
  проверял effective<=3/>=48. Теперь unselected normal/min/max = gold/dark-red/
  purple, selected = orange/red/cyan. Реальные server modifiers сохранены.
  Current column90 вместо68 вмещает `18/100` без ellipsis.
- Выделение: исходный mouse background и3-unit left inset уже совпадали CSS
  (проверено по pixels), сохранены. Не хватало отдельной keyboard-only
  `:focus-visible` рамки1 #d7b548. Добавлен viewport provenance; рамка появляется
  при stats keyboard interaction и скрывается при Tab на footer, mouse-down,
  focus loss и смене owner. Mouse hover не сбрасывает клавиатурный focus.
- Mode: исключённый прежде из reference cards этап теперь использует rounded
  cards, reference palette/14px captions/13.5px copy, heading/intro и Create & enter.
  Стрелки/numpad следуют видимым колонкам: horizontal wrap внутри строки, vertical
  wrap внутри колонки с пропуском отсутствующей последней ячейки. Навигация сама
  не подтверждает режим, semantic IDs/legality остаются production core.
- Decoder: production login replay `REPLY,PLAY,SUCCESS,END,VERSION` воспроизвёл
  точное сообщение об ошибке. Server baseline Login_player после END сразу шлёт
  VERSION и gameplay. Login теперь завершает decoding на END и сохраняет tail
  для следующего owner через `sv_login_take_remaining`; endpoint прекращает
  startup reads на этой границе. До END control semantics сохранены. Полноценный
  gameplay/live-owner integration остаётся этапом C; он не реализован этим fix.

## Linux проверки

- `make -C src -f makefile.sv tomenet-sv -j4` — PASS.
- Production native core body hotkeys, stats repeat, mode four-direction grid:
  strict clang+UBSan PASS. Новый минимальный mode тест подключён в endpoint runner.
- Existing `tests/sv/login.c` с минимальной regression coalesced+later tail,
  too-small output/retry и bytes preservation — UBSan PASS.
- Production SDL rendering: все6 value colors и width18/100;4 комбинации
  keyboard/mouse × footer/choices focus outline; actual native focus event sequence
  — PASS. Это scratch harness существующего renderer, не альтернативное поведение.
- Controlled paced endpoint: exact16266 request bytes неизменны, authoritative
  ACK+END+VERSION одной write и later VERSION следующей write — PASS,
  live-session-handoff. Observer запрещает startup decoder call после END.
  Тестовая учётная запись, loopback peer; реальные персонажи не создавались.
- Финальные renderer frames просмотрены:1920×1200,1024×768 и scale1.5.
  Synthetic view, production renderer software/dummy; не live server acceptance.
- Историческая full matrix не запускалась; Windows отложен по правилу платформ.

Evidence: [decode red](../../evidence/sv-b2-mode-final/decode-red.log),
[endpoint green](../../evidence/sv-b2-mode-final/endpoint-green.log),
[stats render](../../evidence/sv-b2-mode-final/stats-render.log),
[stats focus](../../evidence/sv-b2-mode-final/stats-focus.log),
[mode core](../../evidence/sv-b2-mode-final/mode-core.log).

[Stats keyboard](../../evidence/sv-b2-mode-final/stats-keyboard.png),
[mouse](../../evidence/sv-b2-mode-final/stats-mouse.png),
[minimum](../../evidence/sv-b2-mode-final/stats-min.png),
[scale1.5](../../evidence/sv-b2-mode-final/stats-fractional.png),
[mode wide](../../evidence/sv-b2-mode-final/mode-wide.png),
[mode minimum](../../evidence/sv-b2-mode-final/mode-min.png).
