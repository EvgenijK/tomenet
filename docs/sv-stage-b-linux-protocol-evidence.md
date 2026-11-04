# Stage B — Linux protocol/core evidence, 2026-10-28

Результат: current production-path проверки обновлены; это focused evidence,
а не полная приёмка B-075. Canonical denominator — **54 capability IDs и 162
obligations** (`result`, `lifecycle`, `allocation-checks` у каждого ID).
Данный отчёт исследует 40 IDs owners B-002/006/021/022/023/024; B-001 и B-005
передаются их владельцам. Windows-only `credentials.provider-windows` отложен
решением пользователя, без фиктивного PASS. Остальные canonical IDs и obligations
сохраняются; см. [правило платформ](agents/platform-validation.md).

## Среда и воспроизведение

Linux 6.18.49-1-MANJARO x86_64, Clang 22.1.8, SDL 3.4.16, SDL_ttf 3.2.2,
FreeType 26.6.20, libsecret 0.21.7. HEAD `d9862f29e43905a3dc87ce9b08c0e1970c25094c`;
работа выполнялась поверх исходно dirty tree, не поверх чистого HEAD.
Standalone `src/tomenet-sv` SHA256:
`03d0241fe37d9491b5562db4fac2b52930208058d64757679eaeb4d20ccc5222`.

Команда каждой строки ниже: `python3 -B tests/sv_<suite>_checks.py` из корня.
Runner сам компилирует production sources и применяет существующие sanitizer
flags; contact/live suites вызывают самостоятельный `tomenet-sv` и
`make -f makefile.sv tomenet-sv`. Fixture suites вызывают production
`sv_endpoint_run` с native SDL router, transport, decoder, serializer и models.
SDL dummy/software и SDL_PushEvent/physical translation wrappers в этих suites
проверяют production callers, но не заменяют real-window input/native review.

Исходный sandbox запрещал `socket()` (`EPERM`): эти первые failed attempts
сохранены, не являются production failures. Peer suites повторены с разрешённым
loopback TCP вне sandbox. Логи, exact handshake captures и fingerprints:
`.scratch/stage-b-linux-2026-10-28/protocol/`.
`fingerprints.json` содержит SHA256 168 source/test/fixture/build files, включая
самостоятельный binary; SHA256 manifest:
`40a0418031e33a258f832c22fb5af6329d8bff59446bfde3e7da85a7054e2afc`.
Публичные fixture identities используются только в тестовых локальных профилях.

## Исправление production pong telemetry

На endpoint startup path `sv_login_receive` ранее потреблял `pong!=0` без
обновления telemetry; успешный тест `SvApp` этого caller не подтверждал.
Теперь login хранит bounded 60 samples/latest/average с baseline smoothing,
вычисляет RTT платформенными часами (Linux `gettimeofday`) и не отвечает на pong.
Pregame публикует telemetry только после проверки текущей generation; новая
сессия сбрасывает samples. Неполный пакет не публикует update.

Baseline startup ещё не вызывает gameplay lagometer `Send_ping`: correlation
counter остаётся 0. Это сохранено; unsolicited ping traffic не добавлен.
Signed wire IDs попадают только в bounded baseline index window.
Изменены только `src/client/sv/protocol/login.{c,h}` и
`src/client/sv/session/pregame.{c,h}`. Общий и legacy код не изменялись.

**Ограничение:** пользователь явно отказался от новой regression-проверки для
этой ветви. Тесты не добавлялись и не расширялись. Existing suites подтверждают
отсутствие обнаруженных регрессий; новое pong telemetry behavior отдельно не
считается динамически verified. Windows timer/build остаются deferred.

## Current checks

| Existing suite | Result | Scope / freshness |
|---|---|---|
| `contact` | PASS | core; исходный run |
| `login` | PASS | final source; login-final.log |
| `contact_live` | PASS | final standalone executable; contact_live-final.log |
| `login_live` | PASS | final standalone executable; login_live-final.log |
| `first_session` | PASS | final source; success/disconnect/quit/retry |
| `account_create` | PASS | final source; 8 provider outcomes, OS doubles |
| `account_failure` | PASS | final source; 9 failure/cancel/retry cases |
| `ordinary_create_cancel` | PASS | final source; 18 availability cases |
| `guide_motd` | PASS | final source; both compile orders |
| `name_editor_submit` | PASS | full matrix до scale/telemetry fixes; итоговая выборка из 12 сценариев, две сборки |
| `name_rejection_recovery` | PASS | final source; forbidden/duplicate/unowned/retry/exit/disconnect |
| `birth_complete` | PASS | final source; waiting/partial/ack + END, 16266 exact bytes |
| `birth_single_submission` | PASS | final source; waiting/partial/ack/retry и quit/cancel каждого owner |
| `birth_reply` | PASS | final source; 17 authoritative reply/fault/chaining cases |
| `birth_version` | PASS | final source; 40 baseline version probes, exact captures сохранены |
| `birth_dedicated` | PASS | final source; 5 IDDC/PvP variants × waiting/ack × 2 compile orders |

Все 16 существующих focused suites завершились с exit 0. Итоговые results JSON:
`results.json` (первые sandbox attempts), `host-results.json`,
`extra-results.json`, `final-results.json`, `live-final-results.json`. Full
name/birth choices был скомпилирован до последних scale/telemetry fixes; после
них повторена targeted mouse/viewport/stale/disconnect/unsupported выборка.
M1/account/availability/Guide/name rejection, standalone live и все final birth
handshake suites проверены после обеих production правок. SHA256 всех 168
зафиксированных файлов повторно сверен после suites: drift отсутствует.

## Что проверено по протоколу

- Contact protocol 1/2: независимые literal request/verify bytes, plain/XOR42
  password, star rejection, atomic undersized output. Bytewise contact/setup,
  recommendation/trait setup boundaries, negative version and setup faults.
- Standalone controlled peer: DNS failure, closed socket, timeout, все 15
  rejecting contact statuses, verify/setup rejection, malformed/truncated input,
  старые/новые version layouts, exact login initial/name bytes.
- Login core: complete terminated overview и row fields/flags, bytewise input,
  no partial output, selected-character state, keepalive no ACK, ping echo exact
  tuple/payload один раз. Core `SvApp` дополнительно проверяет send backpressure,
  bounded lag telemetry и session-scoped setup; это supporting evidence, не
  замена B endpoint caller.
- M1/account: authentication только после complete server response, save policy
  после authentication, rejection/cancel/disconnect/retry. Provider doubles
  проверяют production caller policy, а не наличие Linux Secret Service.
- M2: existing suites проверяют all owners, backtracking/quit, оба
  `SV_CLASS_BEFORE_RACE` orders, skipped traits/body, dedicated IDDC/PvP и
  unsupported creation mode; Guide exact placeholder возвращает actual caller.
  Packet interleaving и stale input не создают преждевременный final send.
- Final handshake: independent fixture 16266 bytes; partial reply не даёт
  completion, authoritative ACK+END даёт MOTD/handoff. Rejection/wrong packet,
  disconnect/truncation, retry и chained keepalive/SFLAGS/ping/reply проверяются
  на actual endpoint path. Полный version suite использует 40 explicit probes
  ниже/на/выше baseline gates; synthetic neighbor tuples не выдаются за реальные
  historical releases. Dedicated handshake проверяется в обоих compile orders.

## Карта всех 54 IDs

Для каждой строки evidence относится к конкретному covered behavior в sections
выше. Строка не означает PASS всех трёх canonical obligations. В частности,
`lifecycle` требует отдельного native software/accelerated evidence, а
`allocation-checks` не закрывается одним shared primitive. Canonical
`native-coverage.json`/`native-evidence.json` данным отчётом не повышаются.

| Capability ID | Owner | Focused evidence / applicability |
|---|---|---|
| `capability.connection.select-server` | SV-B-001 | отдельный Linux native owner; данный отчёт не принимает |
| `capability.connection.enter-host` | SV-B-001 | отдельный Linux native owner; данный отчёт не принимает |
| `capability.connection.cancel-host` | SV-B-001 | отдельный Linux native owner; данный отчёт не принимает |
| `capability.input.physical-keys` | SV-B-001 | отдельный Linux native owner; данный отчёт не принимает |
| `capability.input.prompt-navigation` | SV-B-001 | отдельный Linux native owner; данный отчёт не принимает |
| `capability.input.text-edit` | SV-B-001 | отдельный Linux native owner; данный отчёт не принимает |
| `capability.input.confirm` | SV-B-001 | отдельный Linux native owner; данный отчёт не принимает |
| `capability.connection.contact` | SV-B-002 | contact/contact_live/login + lifecycle interleaving; новый pong branch без отдельного теста |
| `capability.connection.contact-failure` | SV-B-002 | contact/contact_live/login + lifecycle interleaving; новый pong branch без отдельного теста |
| `capability.network.keepalive` | SV-B-002 | contact/contact_live/login + lifecycle interleaving; новый pong branch без отдельного теста |
| `capability.network.ping-echo` | SV-B-002 | contact/contact_live/login + lifecycle interleaving; новый pong branch без отдельного теста |
| `capability.network.partial-packet` | SV-B-002 | contact/contact_live/login + lifecycle interleaving; новый pong branch без отдельного теста |
| `capability.network.malformed-packet` | SV-B-002 | contact/contact_live/login + lifecycle interleaving; новый pong branch без отдельного теста |
| `capability.network.server-flags` | SV-B-002 | contact/contact_live/login + lifecycle interleaving; новый pong branch без отдельного теста |
| `capability.credentials.lookup` | SV-B-005 | отдельный credential/provider owner; provider-windows deferred |
| `capability.credentials.provider-linux` | SV-B-005 | отдельный credential/provider owner; provider-windows deferred |
| `capability.credentials.provider-windows` | SV-B-005 | Windows-only; deferred by user, без PASS |
| `capability.credentials.private-input` | SV-B-005 | отдельный credential/provider owner; provider-windows deferred |
| `capability.account.secret-provider-failure` | SV-B-005 | отдельный credential/provider owner; provider-windows deferred |
| `capability.account.restore-secret` | SV-B-005 | отдельный credential/provider owner; provider-windows deferred |
| `capability.account.save-secret` | SV-B-005 | отдельный credential/provider owner; provider-windows deferred |
| `capability.account.enter-name` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.account.cancel-name` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.account.enter-password` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.account.cancel-password` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.account.reject-unencodable-password` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.account.authenticate` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.account.login-rejected` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.character.read-overview` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.character.select-existing` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.character.quit-overview` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.session.read-motd` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.session.disconnect` | SV-B-006 | login/login_live/first_session/account_failure/guide_motd |
| `capability.account.create` | SV-B-021 | account_create/account_failure; provider doubles только на внешней границе |
| `capability.character.create-ordinary` | SV-B-022 | ordinary_create_cancel/name_editor_submit/name_rejection_recovery |
| `capability.character.create-exclusive` | SV-B-022 | ordinary_create_cancel/name_editor_submit/name_rejection_recovery |
| `capability.character.name` | SV-B-022 | ordinary_create_cancel/name_editor_submit/name_rejection_recovery |
| `capability.character.cancel-name` | SV-B-022 | ordinary_create_cancel/name_editor_submit/name_rejection_recovery |
| `capability.character.name-rejected` | SV-B-022 | ordinary_create_cancel/name_editor_submit/name_rejection_recovery |
| `capability.birth.sex` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.race` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.trait` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.class` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.body` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.stats` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.mode` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.quit` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.backtrack-race` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.backtrack-trait` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.backtrack-class` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.backtrack-body` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.backtrack-stats` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.backtrack-mode` | SV-B-023 | name_editor_submit/guide_motd/single_submission/dedicated |
| `capability.birth.complete` | SV-B-024 | birth_complete/birth_reply/birth_version/birth_dedicated/single_submission |

## Ограничения доказательства и передача приёмки

- Новый endpoint pong telemetry branch исправлен, но отдельное dynamic evidence
  отсутствует по прямому решению пользователя о ненаращивании тестов.
- Этот run не заявляет произвольный полный Cartesian product пакетов, surfaces
  и versions. Это ограничение описания evidence, не дополнительный gate:
  приёмка проверяет конкретные obligations и branches исходных tickets.
- Linux native software/accelerated, actual physical keyboard/mouse/input method,
  focus/resize/minimize/restore и human review принимает отдельный native report.
  Secret Service принимает отдельный provider report. Этот отчёт их не подменяет.
- M1/M2 test executables используют production endpoint seam; полный joint
  standalone native flow, canonical per-obligation evidence import, acceptance
  восьми owners и итоговый B-075 остаются решением оркестратора.
- Gameplay/live-session handoff потребителя, real server, profile/resources/DNA
  persistence и Guide implementation остаются за пределами B.
