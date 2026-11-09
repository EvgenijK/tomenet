# C002 — решение о приёмке и техническая сверка

Статус: **accepted**, 2026-11-07, по явному решению пользователя.
Приёмка включает C002 и задачи SV-C002-001–004. [Evidence](sv-c002-evidence.md),
[задачи](tasks/c002/README.md), [spec](sv-stage-c002-spec.md).

## Решение пользователя — 2026-11-07

Пользователь: «тогда подтверждаю приёмку».

C002 принят по представленным реализации, Linux evidence и технической сверке,
включая исправленный birth fixture и успешные `waiting/partial/ack` от 2026-11-07.
Сохраняются описанные ниже границы текущих consumers, diagnostic order `-e`,
проверенной среды и отсрочка Windows/MinGW/Wine. Поздние outcomes других этапов
и общий semantic sweep этим решением не принимаются.

Canonical native-runtime registration не выполнялась: это решение фиксирует
пользовательскую приёмку и не выдаётся за machine-readable registry PASS.
Canonical JSON и исторический planning snapshot остаются без изменений.

## Техническая сверка

Settings core, собственные OPT layers, pre-send character application, startup
overrides/no-autosave и initial audio реализованы в production SV. Linux build,
четыре минимальных core tests, existing login/first-session checks и короткие native
startup/exit observations прошли. Legacy/common source не менялся. Ownership
шести IDs и все20 canonical obligations сохранены без повышения registry статусов.

Initial audio boundary разрешена SV-local startup consumer: actual device,
packs и reporting, без live playback/UI C055/C056. Legacy ignored-buffer defect
не переносится в SV: requested hint применён, queried effective state сохраняется
отдельно. Physical audio, font/rendering matrix и late settings UI не принимаются.

[CLI union](tasks/c002/cli-disposition.md) сверена по каждому baseline branch:
действующие consumers получают реальные overrides, late consumers получают
сохранённые invocation data, CUI исключён ранее утверждённой policy. CLI password
не проходит через CFG/OPT; существующий `-R` request не создаёт отсутствующего
DNA loader C040.15-byte имя не сокращается служебным prefix. `-C` применяется
после выбора endpoint, `-s` действует через единый scale owner, `-e` открывает
requested process log. Ограничение `-e`: diagnostic redirection выполняется
один раз после проверки argv; это записано отдельно, не скрыто под exact parity.

Исторический запуск full-birth fixture 2026-11-05 не дал handshake evidence:
ошибка старого burst choreography воспроизведена также на HEAD до C002.
Follow-up 2026-11-07 исправил только локальную test choreography через настоящий
rendered semantic view и source-backed ожидание normal Quit exit code `0`.
Существующий Linux ASan+UBSan recipe прошёл waiting/partial/ack: первые два
остались `PLAY_PENDING`, только полный authoritative reply дал `LIVE_HANDOFF`.
Literal 16266-byte handshake, early character options, отсутствие premature и
duplicate play, ping, audio и birth outcome assertions сохранены.
[Подробности и raw evidence](sv-c002-evidence.md#ограничения-и-отдельные-результаты).
Это закрывает прежнее ограничение для трёх default cases; version matrix и
соседние suites не запускались. Windows deferred.

Пользовательская приёмка зафиксирована решением выше. Canonical registration
остаётся отдельным existing evidence workflow, не следствием наличия этого документа.
