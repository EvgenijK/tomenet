# Независимое review: pregame keyboard clock / queue ownership

2026-10-30. Scope: credential caller delta `.scratch/credential-producer/before-gate.c`, расширение `.scratch/pregame-clock/before-{run,endpoint,birth}.c`, новый SV-local `input/pregame-events.h`, минимальная production-core проверка native endpoint. Reviewer production source/tests не изменял. SHA проверены командой `sha256sum -c .scratch/pregame-clock/final.sha256`: все OK.

## Результат

Открытых actionable замечаний нет.

Пользовательский trace `.scratch/credential-focus/keyboard-entry-failing.log` подтверждает причину: Backspace/Tab поступали с правильными logical key/scancode, repeat=0, фокусом окна и активным text input, однако свежие события имели frozen backend timestamp раньше scene start и отбрасывались до interaction handler. Это отдельная подтверждённая причина; ранее добавленный UNKNOWN-key fallback её не объяснял.

- Credential entry останавливает text producer, выполняет Pump+flush только KeyDown/KeyUp/TextInput/TextEditing, затем снова запускает text input. Clock guard остаётся у pointer gestures; свежие keyboard/text после барьера не сравниваются с SDL tick clock.
- Тот же механизм ограничен pregame: list startup/re-entry, list/manual phase changes и provider failure→manual, contact generations/login transitions, name start/cancel/submit, birth entry/owner changes и Guide open/close. Барьера каждый кадр нет.
- Открывающий key/text, уже стоящий в очереди, удаляется до передачи управления новому owner. При birth selection→confirm барьер применяется после смены owner; текущий gesture не обрабатывается повторно следующим шагом. Repeat guards, generation identity/active checks и model legality сохранены.
- Queue helper не меняет event timestamps, не перенормализует глобальный поток, не затрагивает gameplay native-input. Pointer clock guards остаются прежними.
- Flush удаляет только keyboard/text диапазоны: USER/callback, window-close/QUIT, render/reset и прочие события остаются в очереди. Close обрабатывается caller до session pointer gate. Production native endpoint regression сохраняет USER event, удаляя opening Enter/text; повторный запуск `.scratch/pregame-clock/native-endpoint` PASS.
- Safe reconnect lifecycle и private credential buffers не изменены этим delta. Очередь старой input ownership отделяется барьером, сетевые/модельные старые поколения по-прежнему отделяются generation identity.

## Проверенные evidence

- `.scratch/credential-producer/caller-red.log`: свежие fixed-key events поллятся, но handler не вызывается; Backspace/Tab не срабатывают. `caller-green.log`: оба действия проходят, opening queue не протекает.
- `.scratch/pregame-clock/birth-boundary-green.log`: queued Enter/text удалены, USER сохранён, один letter gesture не подтверждает следующий owner.
- `.scratch/pregame-clock/grid-green.log`, `guards-green.log`: геометрия/guards сохраняются.
- `.scratch/pregame-clock/name-birth-green.log`: production name→birth→play acknowledgement с свежими keyboard events, имеющими timestamp до generation; точное совпадение 16266 handshake bytes и live-session-handoff.

Исправленный физический Wayland сценарий пользователь ещё проверяет; этот report не объявляет user acceptance и не подменяет её синтетическими event replays.
