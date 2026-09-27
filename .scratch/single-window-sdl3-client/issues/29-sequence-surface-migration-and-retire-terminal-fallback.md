# Sequence surface migration and retire terminal fallback

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 09, 28

## Question

Применить утверждённый [Acceptance contract](../acceptance-contract.md), решение — [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md#answer). Пользователь требует отдельного определения этапов: для каждого зафиксировать capabilities, обязательные checks и известные ограничения; pending capabilities следующих этапов не блокируют текущий, ранее принятые получают regression checks. HTML предоставляет визуал и не блокирует native implementation своей полнотой. Reference — одна машина, Linux/Wine/Windows VM; software rendering обязателен, XHTML/intensive-load/long-soak/global-memory-ceiling не возвращаются как gates. Final coverage и zero terminal fallback остаются обязательными.

Какой implementation sequence переносит все capability-manifest outcomes из development-only terminal fallback в semantic surfaces и presentation state, сохраняя runnable Linux-first/early-Windows checkpoints, и какие evidence gates запрещают завершение каждой фазы с потерянным behavior flow?

Использовать geometry/surface allocation из [Specify surface layouts and responsive rules](21-specify-surface-layouts-and-responsive-rules.md), atomic baseline/consumer ledgers и acceptance contract. Определить вертикальные slices через decode→model→surface→input/fallback→persistence/platform evidence, порядок server documents/special stores/pregame/system scenes, временные fallback ownership/observability rules и final zero-fallback exit gate. Не превращать legacy `Term` cells в permanent model и не восстанавливать утраченные modern object files. Реализация, удаление artifacts и release остаются вне planning ticket.


## Comments

### Round 1 — proposed, awaiting user answers — 2026-09-20

Применены wayfinder, grilling, domain-modeling и codebase-design. Все blocking tickets resolved; это единственный оставшийся открытый вопрос карты. Ниже предложения, не утверждённый implementation contract.

- Q1: порядок checkpoints: (A) отдельный target, one-window shell, model/router/renderer seams и synthetic vertical slice, Linux/MinGW builds; (B) real connection/login/MOTD/character selection/creation, map/HUD/movement/messages и exit/reconnect; (C) inventory/equipment/items/targeting/spells/обычные магазины и соответствующие prompts/macros; (D) остальные information/social/local/server documents и special stores/canvases; (E) полнота settings/import/resources/audio/files/OS integrations; (F) final coverage, packaged runtime и zero fallback. Каждая capability получает необходимые persistence/input/error/platform части уже в своём этапе; E не откладывает эти prerequisites. Точная atomic-ID allocation и checks следуют после выбора порядка.
- Q2: MinGW target с A, Wine smoke каждого этапа; реальные Windows 10/11 VM checks на B, E, F и раньше при изменениях platform-specific code. Linux software rendering с первого rendered slice; Windows software rendering при первом runtime. Это предложение конкретизирует ранее принятое early-Windows обязательство.
- Q3: development fallback только для явно перечисленных ещё не принятых capabilities; для каждого entry — replacement stage и безопасный diagnostic ID. Принятый native flow не может молча уйти в fallback; такой переход проваливает его regression check. Content/secrets в диагностику не попадают. Final build исключает fallback adapter; temporary terminal state не становится permanent model.

После ответов уточнить полный состав этапов, обязательные checks/ограничения, переходы native↔fallback и final removal evidence. Тикет остаётся claimed; решения и Answer пока отсутствуют.

### Round 1 — confirmed — 2026-09-20

Пользователь: «1 - да, 2 - ок, 3 - да». Q1–Q3 приняты полностью: порядок A–F с prerequisites внутри своей capability, MinGW/Wine/Windows checkpoints и ограничение fallback будущими непринятыми capabilities. Детальный состав/проверки этапов и handoff между native/fallback уточняются следующим раундом; тикет ещё не resolved.

### Round 2 — proposed, awaiting user answers

Draft: [Implementation sequence](../migration-sequence.md). Q4 proposes full atomic manifest/native ledger and A–F allocation as an A exit gate (the files currently do not exist); Q5 proposes exclusive router-owned flow/context handoff with explicit fallback entries and preserved queue/reply/parent lifecycle; Q6 proposes the per-stage checks/limits in the draft, disabling fallback per accepted flow, all-native E and no-linkage/final coverage F. No runtime evidence is claimed.


### Round 2 — confirmed — 2026-09-20

Пользователь: «4 - ок, 5 - да, 6 - ок». Q4–Q6 приняты полностью: заполнение атомарного реестра и распределение A–F до завершения A; явные переходы native/fallback через единый input router; проверки/ограничения этапов, отключение fallback по мере принятия native flows, полностью native E и final no-linkage/coverage/package gate F. Вместе с Q1–Q3 это завершает согласование implementation sequence.

## Answer

Утверждён [Implementation sequence](../migration-sequence.md): контракт состава, проверок, ограничений и переходов A–F, применяющий ранее согласованный [Acceptance contract](../acceptance-contract.md). Этапы последовательно дают runnable foundation, session/game screen, game actions, information/server surfaces, integration completeness и final acceptance. Необходимые input/persistence/platform prerequisites входят вместе со своей capability; принятые outcomes проверяются на регрессии.

До завершения A требуется создать полный атомарный manifest/native ledger и распределить outcomes по этапам на основе завершённых inventories. Сейчас эти artifacts не реализованы; наличие family inventories не выдаётся за готовый atomic registry. MinGW поддерживается с A, Wine проверяется на каждом этапе, реальные Windows 10/11 — на B/E/F и раньше при platform-specific changes.

Development-only fallback разрешён только явным будущим flows с владельцем и этапом замены; input router сохраняет queue/request/parent semantics. Принятый native flow не возвращается в fallback. E проходит все сценарии с отключённым fallback; F исключает его linkage и требует актуальное полное evidence из целевых архивов. Permanent formatted documents/canvases не являются fallback.

Q1–Q6 подтверждены пользователем. Новых нерешённых planning-вопросов не выявлено. Реализация, registry/checker population, VM provisioning, runtime acceptance, очистка old modern objects и выпуск остаются следующим этапом за пределами карты.
