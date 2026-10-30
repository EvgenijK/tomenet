# Stage B — Linux native evidence, 2026-10-28

Текущая приёмка ограничена Linux по [правилу платформ](agents/platform-validation.md).
Это машинный controlled-peer прогон production executable и просмотр PNG агентом;
он не является human review и не подтверждает OS IME.

## Production correction

На X11 с `SDL_VIDEO_X11_SCALING_FACTOR=1.5` воспроизведён промах по
отрисованной строке существующего персонажа: controlled peer не получил выбор.
`endpoint-run.c` теперь переводит window mouse button coordinates в UI units
через `display_scale / pixel_density` перед production native adapters.
Birth viewport получает output height / display scale; размеры resize events
также переводятся в UI units. Mouse wheel использует только scroll delta.
Исправление находится в SV; legacy и общие модули не менялись.

## Evidence and scope

Проверялся Linux X11 поверх доступного desktop, SDL3 3.4.16,
SDL3_ttf 3.2.2, FreeType 26.6.20. Тестовый peer слушает только loopback;
аккаунт/пароль синтетические, профиль отдельный, Secret Service намеренно
недоступен в этом прогоне. Рендереры: software и OpenGL.

Существующий временный `/tmp/sv-b025-evidence/native.py` повторён с настройками
renderer/output directory. Дополнения driver ограничены масштабом XTest click,
вводом мышью вместо клавиатуры, capture password/focus и восстановлением XKB
Latin group после focus (KDE может восстановить раскладку при focus).
Это driver настоящего executable, не отдельная реализация поведения.

| Проверка | Результат |
|---|---|
| `python tests/sv_endpoint_checks.py` | PASS: production editor, SDL adapter normal/sticky, physical bytes, credentials, metaserver parser и native scene, UBSan |
| Software, scale 1, existing / creation / new-account | PASS, executable `a8221eafafac5e7c02c584d74d08bf6258e2b07eb19bff530c0e6e864c75d358`, до scale fix |
| Software, controlled scale 1.5, existing mouse selection | PASS, executable `56b4fdbda5d03300ec4d02b77ef9075ff6c6ea4fde526e315c6abef7e7b66ba4`, после mouse fix, до resize-event addition |
| Software, controlled scale 1.5, creation mouse path | PASS, executable `e14d16dc9e8206359cc92a134ab4cf1fd4e9478cc20f2b3b260a38fc872a9ba1`, полный scale fix |
| OpenGL, scale 1, existing / creation | PASS, executable `56b4fdbda5d03300ec4d02b77ef9075ff6c6ea4fde526e315c6abef7e7b66ba4`, до resize-event addition |
| OpenGL, scale 1, existing | PASS, executable `e14d16dc9e8206359cc92a134ab4cf1fd4e9478cc20f2b3b260a38fc872a9ba1` |
| Final combined executable, software scale1, M1 existing mouse/MOTD/handoff/resize/minimize/restore | PASS, `03d0241fe37d9491b5562db4fac2b52930208058d64757679eaeb4d20ccc5222` |
| OpenGL final creation с дополнительным root-focus/restore | Не принят: draft уже до focus transition пуст с encoding error; layout driver не добился ASCII ввода, peer закрылся. New-account в этом запуске не выполнен |

Scale-1.5 creation использует мышь для new-character action, caret имени,
sex/class/race/trait/body/stats/mode choices; Return подтверждает выбор.
Controlled peer сравнивает production creation/play request с literal
`tests/sv/fixtures/birth-current.hex` (16266 bytes), затем выдаёт partial/full
ACK и MOTD. Handoff не отправляет gameplay bytes при Up/period/Enter;
startup keepalive допускается. Выполнены resize, minimize/restore.
В final OpenGL попытке draft был пуст с encoding error уже до
root-focus/restore; после focus экран остался тем же. Это не доказательство
production focus regression. Сохранены PNG и failure; сценарий не принят.
Это controlled X11 content-scale override, а не физический HiDPI монитор,
display migration или pixel-density != 1.

Артефакты: `/tmp/sv-b-linux-native/` (PNG, `.wire`, per-route log и summary),
`/tmp/sv-b-linux-native-driver.py`, `/tmp/sv-b-linux-native-mouse.py`,
`/tmp/sv-b-linux-native-scaled.py`. Они локальные временные; durable conclusion
и executable identities записаны здесь. Ранние failures сохранены отдельно:
sandbox denied XOpenDisplay; non-1 failure до fix; неверная раскладка временного
driver, которая закономерно дала encoding error; OpenGL new-account был
прерван, когда driver продолжал получать только keepalive.

Финальный combined executable включает последующие protocol pong-telemetry
изменения. Для freshness после их сборки повторён только M1 existing smoke
(`/tmp/sv-b-linux-native/combined-final-m1/summary.json`); прежние creation
и OpenGL observations относятся к явно указанным выше executable identities.

## Remaining acceptance and observation limits

- Actual OS IME composition/commit/cancel не проверялись: в доступном
  окружении не найдены ibus/fcitx executables или services. Это ограничение
  наблюдения, а не самостоятельный blocker. Требование input method не
  означает обязательную установку ibus/fcitx или отдельный сценарий Chinese
  composition. Injected `SDL_EVENT_TEXT_INPUT` не является OS IME evidence.
  Применимые native text input, byte/edit/focus contracts сохраняются.
- Human native review на конкретном production build, включая читабельность,
  focus/caret/selection и private field masking. PNG агента не заменяют человека.
- Actual scaled-display migration/pixel-density != 1 остаются непроверенными;
  controlled scale1.5 теперь подтверждает исправленный mapping на X11.
- Secret Service provider acceptance принадлежит B-005 и отдельному evidence.

Windows/MinGW/Wine отложены и не являются оставшимся Linux blocker.

## Review reproduction

Сохранённые PNG показывают конкретные экраны и executable hashes выше.
Для повторения controlled run из корня проекта:

```sh
SV_NATIVE_OUTPUT=/tmp/sv-b-linux-native/review-software SV_NATIVE_ROUTES=existing,creation,new-account python /tmp/sv-b-linux-native-driver.py
SV_NATIVE_OUTPUT=/tmp/sv-b-linux-native/review-opengl SV_NATIVE_RENDERER=opengl SV_NATIVE_ROUTES=existing,creation,new-account python /tmp/sv-b-linux-native-driver.py
SV_NATIVE_OUTPUT=/tmp/sv-b-linux-native/review-scale15 SV_NATIVE_SCALE=1.5 SV_NATIVE_ROUTES=creation SDL_VIDEO_X11_SCALING_FACTOR=1.5 python /tmp/sv-b-linux-native-mouse.py
```

Driver нажимает клавиши автоматически и завершает приложение; его результат —
машинная проверка. Человек может оценить сохранённые screen captures, но
human review применимого native text input и физического ввода требует
отдельного интерактивного наблюдения; оно здесь не присвоено. Настоящий OS IME
можно проверить на desktop с настроенным input method, но отсутствие такого
сервиса само по себе не блокирует приёмку.
