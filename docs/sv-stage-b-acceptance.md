# Stage B — Linux cumulative acceptance, 2026-10-28

Решение gate **SV-B-075: accepted by user**, 2026-10-28. Этап B принят
по прямому решению пользователя без ожидания оставшихся проверок.
Production startup flow реализован; не все Linux проверки выполнены. Windows/MinGW/Wine
отложены по [правилу платформ](agents/platform-validation.md); они не получают
PASS и не блокируют это решение.

Текущий presentation follow-up: [этап B2 — интерфейс по веб-макету](sv-stage-b2-spec.md).
B остаётся принятым; B2 планирует перенос дизайна без переоткрытия снятых проверок.

## Scope и ownership

Только native startup: endpoint/contact → account → existing character либо
new slot/name/birth → authoritative ACK/MOTD → live-session handoff.
Real server, post-MOTD startup, FILE/Lua/profile и gameplay принимаются после B.

Canonical ledger сохраняет 54 B IDs, 162 evidence obligations и `pending`
для всех 54 записей. Наличие успешных focused suites не заменяет current
per-obligation evidence; canonical claims не повышались.
Windows-only `capability.credentials.provider-windows` временно deferred.

| Owner | IDs | Текущий evidence / остающаяся граница |
|---|---:|---|
| B-001 | 7 | Native endpoint/input; применимый способ ввода и human input review требуют подтверждения. |
| B-002 | 7 | Production contact/protocol controlled peer; pong telemetry исправлена, отдельный новый тест отклонён пользователем. |
| B-005 | 7 | [Actual Secret Service](sv-b005-evidence.md#linux-freshness-review--2026-10-28): binary roundtrip, replacement/deletion, locked denial; interactive unlock/cancel/refusal и full confirmed caller с actual persistence pending. Windows provider deferred. |
| B-006 | 12 | Existing-account/character controlled peer; native/provider требования producers сохраняются. |
| B-021 | 1 | Account creation/failure production path; actual provider и human native review pending. |
| B-022 | 5 | Slot/name/rejection/recovery production path; native current evidence не равно complete acceptance. |
| B-023 | 14 | Choices/back/cancel/version/build-order paths; native/input требования остаются применимыми. |
| B-024 | 1 | Exact play bytes, authoritative replies, ACK/MOTD gate; полная native acceptance pending. |

M1/B-020 и M2/B-025 имеют нулевое ownership; они не принимают producer IDs
по одному happy path и остаются pending до исполнения обязательств owners.

## Planning snapshot и cumulative A

`coverage.json` ранее хранил hashes до post-B reallocations из commits
`9a0c05752` и `1b404a6ae`. Перед обновлением hashes проверено:
все 54 capability objects и все 54 allocation rows B совпадают с
`9a0c05752^`; старые hashes совпадают с файлами той revision. Новый snapshot
сохраняет reviewed set digest, obligations, sources, ownership и allocation
hashes. Canonical файлы не изменялись. Статус planning приведён к
`implemented-linux-acceptance-pending`.

`check-plan.py` и 14 existing planning regression tests прошли после refresh.
Свежие existing A production ASan/UBSan checks: architecture, messages,
request и lifecycle — PASS, без отключения LeakSanitizer. Первые sandbox runs
падали из-за ptrace restriction LSan; идентичные host runs прошли.
Логи и command/exit/time/SHA-256 сохранены в
`.scratch/stage-b-linux-2026-10-28/acceptance/verified-core.json`.
Native A regression matrix и финальные build fingerprints зафиксированы ниже.

Разрешение пользователя на расширение проверок ограничено existing vault core
тестом replacement/deletion/locked. Предложенная отдельная regression-проверка
startup ping telemetry отклонена: production исправление проверяется имеющимися
suites, новое тестовое поведение не добавляется.

## Проверки, оставшиеся до решения пользователя

- Current human native review применимого способа ввода и keyboard,
  mouse, focus, resize/minimize/restore, private fields и ошибок.
- Actual provider-owned unlock/cancel/refusal с отзывчивыми UI/network,
  late completion handling и successful persistence через server-confirmed
  production caller. Headless locked denial не заменяет interactive prompt.
- Сведение current evidence к конкретным применимым canonical obligations.
  Отдельной динамической проверки новой pong telemetry ветви нет по решению
  пользователя; existing suites подтверждают отсутствие найденных регрессий.
  Произвольный полный Cartesian product не добавляется как новый gate.
- Актуальные evidence fingerprints после последних production исправлений.

Список не сокращает обязательства implementation tickets. Ни один неисполненный
Linux пункт не объявлен выполненным из-за Windows deferral или проходящих core
checks. Требуемая human review не заменяется автоматическим просмотром PNG.


## Final combined build и native A regressions

Production `src/tomenet-sv` SHA-256:
`03d0241fe37d9491b5562db4fac2b52930208058d64757679eaeb4d20ccc5222`.
На этой сборке existing Linux native A matrix прошла **12/12**: HP, messages,
request, lifecycle, geometry и timing для software и OpenGL. Timing включает
negative delayed-frame control; его предусмотренный отказ проверен wrapper,
а не записан как успешное latency measurement. Geometry — virtual output
scales, без заявления physical monitor acceptance. Native A scenario
проверяет synthetic foundation и не заменяет B startup acceptance.
Команды/exit codes/durations/log SHA-256/executable hash:
`.scratch/stage-b-linux-2026-10-28/acceptance/native-a.json`.

Scale mismatch B исправлен только в `src/client/sv/endpoint-run.c`;
[Linux native report](sv-stage-b-linux-native-evidence.md) сохраняет pre/post
build identities, non-1 mouse creation PASS и не принятые driver/layout
attempts. Ordinary OS layout переключение не считается проверкой IME.


## Уточнение input-method scope

Исходные B-001/B-025/B-075 требуют `input method`, но не предписывают
установку ibus/fcitx или отдельную китайскую/японскую IME matrix. Объявление
отсутствия этих программ самостоятельным blocker было слишком строгой
интерпретацией при текущем review и исправлено. Actual IME не проверялось;
это limitation конкретного прогона, а не новый обязательный gate. Применимые
production text-input, encoding, editing, focus и human review требования
сохраняются.


## Graphify

После изменений production и core test выполнено
`.graphify-venv/bin/graphify update .`: 17604 nodes, 45986 edges. Обновлён
code graph. Экстрактор сообщил о 77 файлах с syntax errors/partial extraction
и 150 файлах без extracted symbols; graph navigation имеет эти ограничения.
Semantic doc graph этим code-only update не обновляется.


Финальные production/test fingerprints сохранены в
`.scratch/stage-b-linux-2026-10-28/acceptance/final-fingerprints.json`.
Копии actual provider logs: `.scratch/stage-b-linux-2026-10-28/provider/`.
Final combined M1 native smoke на том же binary — PASS: mouse character
selection, peer MOTD, handoff, resize/minimize/restore и отсутствие gameplay
send; [native report](sv-stage-b-linux-native-evidence.md).


## Итог protocol/core прогона

[Protocol/core report](sv-stage-b-linux-protocol-evidence.md): **16 existing
focused suites PASS**, включая 40 version probes, 17 authoritative reply
исходов и 20 dedicated scenarios в обоих build orders. Final targeted endpoint
checks повторены после production исправлений. Полный name/birth suite
прошёл до этих двух последних исправлений; current targeted rerun и отдельные
final-source birth suites отмечены в исходном отчёте. 168 source/test/fixture/
build fingerprints перепроверены после suite runs: drift 0.

До решения пользователя B-075 оставался **pending**, главным образом из-за
actual interactive provider/full-caller persistence и human native review. Успешные automated
проверки не присваивают эти результаты человеку или OS provider.


## Решение пользователя — 2026-10-28

После рассмотрения результатов пользователь прямо распорядился:
«не ожидаем оставшихся тестов и принимаем».

Этап B, все восемь implementation owners (B-001/002/005/006/021/022/023/024),
M1/B-020, M2/B-025 и итоговый B-075 **приняты** на текущей Linux сборке
`03d0241fe37d9491b5562db4fac2b52930208058d64757679eaeb4d20ccc5222`.
Оставшиеся проверки сняты с текущего acceptance gate этим решением;
дополнительные запуски не требуются для закрытия B.

Это включает ранее оставшиеся human native review, интерактивные Secret
Service unlock/cancel/refusal, full-caller actual persistence и прочие
неподтверждённые пункты предыдущего review. Они не выполнены и не получают PASS.
Отдельная новая ping telemetry regression ранее также отклонена пользователем.
Новых обязательных follow-up задач или блокеров этапа C это решение не создаёт.
Windows/MinGW/Wine сохраняют ранее установленный deferred статус.

Статусы owner/milestone/gate tickets, stage index/spec и planning coverage
согласованы с решением. Canonical capability ownership, allocation и доказательные
`evidenceStatus`/records не переписаны: пользовательская приёмка этапа не
подменяет отсутствующие измерения или machine-validated evidence. Все выше
приведённые pending observations относятся к состоянию до этого решения.
