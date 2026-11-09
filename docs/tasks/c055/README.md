# C055 — Audio: live preference effects

Статус: planned, 2026-11-09. Компактное обязательное дополнение к C055,
не полный план всех audio outcomes.

## Production seam и работа

[C049](../c049/README.md) связывает доступные audio consumers с option values
и `SV_PREF_OPTION` occurrences. `SvPreferences` сохраняет exact slots, source,
`in_game`/generation и каждое assignment, включая прежнее значение; current
startup audio owner — `src/client/sv/settings/audio.c`. При реализации C055
подключить ещё отсутствующие live audio recipients к тому же application/session
контракту через реальные audio commands/state, без второго PRF parser.

Сохранить source-defined branches `src/client/c-util.c:check_immediate_options`:
`shuffle_music`/`play_all` требуют refresh и при same-value assignment, но лишь
при исходных `music_next == -1` и in-game gates. Остальные paging/SFX/music keys
имеют собственные потребители и availability; не заменять их универсальным
reinitialize и не генерировать sound при каждом assignment. При выполнении
своего live audio lifecycle C055 завершает соответствующий combined
preference→audio flow, в том числе startup intent, удержанный C011/C014.

Actual unavailable/disabled/no-device результат сообщается по audio contract;
requested/effective state не подменяется успешным parser snapshot. Отмена/смена
сессии исключает stale refresh, redraw не повторяет effect. Общий consume/clear
согласовать с message/action recipients. Это обязательство C055, не новый
prerequisite C049→полный C055 и не gate уже проверенных initial/current consumers.

## Минимальные Linux проверки и завершение

Через настоящий live audio owner применить нужную X/Y запись и same-value
повтор, проверить actual refresh/playback decision и исходные blocked/allowed
gates. Пройти только относящийся unavailable/disabled/device и generation/cancel
case действующего consumer; не создавать новую audio matrix или mock runtime
ради PASS. Проверять фактический результат production audio path, exact slot/
event order и requested/effective distinction, без CFG/OPT autosave.

Связка завершена при подключённом live recipient и реальном combined flow
с C049/preference occurrences; parser, retained effect или metadata pack load
не заменяют audio execution evidence. Необходимые corrections выполняются C055.

C004 принят и завершён решением пользователя 2026-11-09. Эта связка и необходимые
исправления обязательны своему позднему этапу, не переоткрывают C004 автоматически.
Его три IDs, conditions и девять obligations сохраняют canonical allocation
и statuses; [C004 snapshot](../c004/coverage.json) не переписывается. Evidence
хранится здесь; backlink в C004 optional для трассировки, не gate его приёмки.
Только необходимый минимум Linux production core checks по [AGENTS](../../../AGENTS.md),
расширение согласовать; Windows/MinGW/Wine deferred. Новые suites, full plans,
harness и общие матрицы не вводятся. Реализация и PASS этой связки не заявлены.
