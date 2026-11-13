# C004 — завершённый этап и последующая интеграция

Статус: **accepted**, 2026-11-09. По решению пользователя C004 и задачи 001–004
завершены сейчас, оставшиеся доработки и проверки закреплены в следующих этапах.
Приёмка явно подтверждена пользователем: «этот этап считаем принятым».
[Решение](../../sv-stage-c004-acceptance.md), [Linux evidence](../../sv-c004-evidence.md),
[спецификация](../../sv-stage-c004-spec.md), [source contract](source-contract.md),
[coverage snapshot](coverage.json).

| Ticket | Реализованный результат | Owned outcomes | Статус |
|---|---|---|---|
| [SV-C004-001](SV-C004-001-preference-core.md) | Один SV-local PRF interpreter, данные и load effects | `preferences.load` | accepted |
| [SV-C004-002](SV-C004-002-includes.md) | Owner-aware includes, diagnostics и continuation | `preferences.include-failure` | accepted |
| [SV-C004-003](SV-C004-003-bootstrap.md) | Startup/global/character wiring и lifecycle | `preferences.bootstrap` | accepted |
| [SV-C004-004](SV-C004-004-evidence.md) | Linux core evidence и закрепление последующей работы | 0 IDs | accepted |

Production owner — `settings/preferences.[ch]`, wrappers — `settings/options.[ch]`,
assembly/lifecycle — `login/native-run.c` и `login/flow.c`. Build и минимальные Linux
checks прошли. Parser не дублируется для startup, explicit load и тестов;
legacy/common не менялись. Сохранены exact aliases, source owners, ordinary
continuation, prepend action queue и encoded-prefix1023/raw retention.

Snapshot сохраняет три exact canonical rows, девять obligations и hashes источников.
Каждый ID имеет одного исторического owner в C004. Новое решение переносит работу
по интеграции и её проверке, не переименовывает IDs и не меняет canonical allocation,
conditions/prerequisites или registry statuses. Full flow PASS по parser evidence
не заявлен.

## Самостоятельные обязательства следующих этапов

Документ своего этапа содержит implementation scope, минимальные production-path
проверки и условия завершения. Эта таблица — навигация, не обязательный общий
verification список и не замена документов владельцев.

| Этап | Документ | Обязательная последующая работа |
|---|---|---|
| C005 | [План C005](../c005/README.md) | Effective keymap/S semantics и применение PRF mappings ввода |
| C006 | [План C006](../c006/README.md) | Effective map font PRF, порядок layers/reset/reload/fallback |
| C007 | [План C007](../c007/README.md) | Graphics PRF/base/subsets/masks и реальные map consumers |
| C011 | [План C011](../c011/README.md) | Session preparation и передача app preferences/one-shot effects без потери |
| C014 | [План C014](../c014/README.md) | Handoff/generation/once-only delivery, retry/cancel/quit |
| C016 | [План C016](../c016/README.md) | Реальный normal/chat route для `#:` без повторов |
| C044 | [План C044](../c044/README.md) | Settings explicit load/dirty/Save с правильным owner |
| C045 | [План C045](../c045/README.md) | Immediate X/Y effects options ввода, сообщений и alerts |
| C046 | [План C046](../c046/README.md) | Immediate X/Y effects HUD/lighting/visibility |
| C047 | [План C047](../c047/README.md) | Immediate X/Y effects map/animation/movement/targeting |
| C048 | [План C048](../c048/README.md) | Immediate X/Y effects item/session options |
| C049 | [План C049](../c049/README.md) | Immediate X/Y effects audio/client-integration options |
| C050 | [План C050](../c050/README.md) | Реальные alert consumers загруженных option values и source-defined gates |
| C051 | [План C051](../c051/README.md) | Data-only staging, no execution и linked import group failure |
| C052 | [План C052](../c052/README.md) | Macro precedence, later reload/body и actual arcade selection |
| C053 | [План C053](../c053/README.md) | Реальное исполнение macro/load actions, completion/waits/cancel |
| C054 | [План C054](../c054/README.md) | Stateful explicit macro load/save UI и mixed records |
| C055 | [План C055](../c055/README.md) | Реальные live audio refresh/option consumers после C049 |

C004 принят без ожидания этих работ. Их implementation/evidence обязательны
в этапах-владельцах; найденные проблемы исправляются там и не переоткрывают C004
автоматически. Дополнение сводки C004 разрешено для трассировки, не является gate.
Поздние этапы остаются planned и не получают фиктивную приёмку.

Только минимальные SV-local изменения и необходимый минимум Linux core checks;
расширение набора проверок согласовывается по AGENTS. Windows/MinGW/Wine deferred.
Новые возможности улучшить legacy/common записываются отдельно в
[список улучшений](../../sv-improvements.md).
