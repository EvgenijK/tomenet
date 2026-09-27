# HTML gap ledger: атомарное покрытие и проверяемые claims

Исследование локальных первичных источников, 2026-09-14. Это спецификация предлагаемого ledger/checker, а не существующая реализация. Решение [Govern the cross-repo capability manifest](../issues/07-govern-the-cross-repo-capability-manifest.md) обязательно; [Inventory the complete behavior baseline](../issues/01-inventory-behavior-baseline.md) определяет полный native scope.

## Вывод

Нужен отдельный нормализованный consumer ledger HTML с полной явной строкой для каждой active capability canonical manifest. Строка связывает independently testable outcome с документированными UX slices, реальными entrypoints/modules, contexts/bindings и evidence. `missing`, `planned`, `prototype-complete` описывают implementation coverage; отдельный claim `ux-approved` требует явного человеческого решения. Ни один HTML claim не доказывает protocol/native parity. Checkbox или наличие JS-файла не заменяют evidence полного flow.

Сейчас canonical `docs/capabilities/manifest.json`, его schema и HTML ledger не найдены. Baseline содержит 36 семейств, а не завершённый набор canonical атомов (источник: baseline в коммите `87ead6ff5`, раздел «Verifiable capability inventory»). Полное перечисление mappings возможно после атомизации остальных baseline исследований; это ограничение исходных данных, а не причина откладывать контракт checker. Новые active атомы всегда импортируются с `missing`, без автоматического inheritance coverage от семейства или replaced ID.

## Что действительно есть в prototype

Все ссылки ниже указывают на прочитанное рабочее дерево `/home/svechnik/Projects/github_site/tomenet_interface`; HEAD `b433fa484d82315f8900d268a2917a78aaff1f79` сам по себе не фиксирует незакоммиченные данные. Native HEAD `4211671279ff820575c32239272bca68cf8762f7`; поведенческие inventory — исследовательские источники, при уточнении следует переходить к указанным там C owners.

| Наблюдение | Первичный источник | Следствие для ledger |
|---|---|---|
| MENUS ограничен post-login обычным игроком; C/network интеграция исключена | [MENUS.md](/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:3), строки 3–15 | Исключённые pregame/admin/platform/native capabilities всё равно получают явный `missing` с причиной; scope документа не сокращает active manifest |
| HOTKEYS `[x]` означает штатный normal keyset и минимальный законченный visual flow; roguelike/slash/admin/OS исключены | [HOTKEYS.md](/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:7), строки 7–19 | Checkbox — source assertion о конкретном slice, без UX approval и без полного input-language parity |
| MENUS DoD требует normal/empty/boundary fixtures, loading/unavailable/error, cancel/confirm/result/focus, fallback/resize/docs | [MENUS.md](/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:593), строки 593–608 | Основа visual acceptance matrix, но не доказательство прохождения сценариев |
| Ghost powers `[x]` — только unavailable-строка, без U/child/effects; HOTKEYS U отсутствует | [MENUS.md](/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:332), [HOTKEYS.md](/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:93) | Полный ghost-use outcome остаётся missing; «показать unavailable» может иметь отдельное slice evidence |
| ghost-powers module управляет demo-state; activation unavailable ничего не выполняет | [ghost-powers.js](/home/svechnik/Projects/github_site/tomenet_interface/js/features/ghost-powers.js:2), строки 2–11; [abilities-window.js](/home/svechnik/Projects/github_site/tomenet_interface/js/features/abilities-window.js:64), строки 64–95 | Имя модуля не означает реализации игрового flow; placeholder не terminal result требуемого outcome |
| Шаг `[x]` локальный, run/pickup/repeat/doors/stairs отсутствуют; Target/current-target/direction transitions частичны | [HOTKEYS.md](/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:69), строки 69–116, 163–171 | Разделять movement atoms, hostile/friendly/acquired target и возобновление исходного action; общий cell selector не покрывает все |
| Screenshot в Locate/Look только prototype message, без файла | [HOTKEYS.md](/home/svechnik/Projects/github_site/tomenet_interface/HOTKEYS.md:141), строки 141–155; [abilities-window.js](/home/svechnik/Projects/github_site/tomenet_interface/js/features/abilities-window.js:130) | Проверять требуемый artifact/side effect, а не текст «saved» |
| GameMenu выполняет только messages/mini-map/players-online; unavailable entry просто поглощает activation | [game-menu.js](/home/svechnik/Projects/github_site/tomenet_interface/js/features/game-menu.js:14), строки 14, 85–95 | Наличие меню/ссылки не покрывает Knowledge, stores, settings, session flows |
| Party fixture не меняет membership; форматированные playerinfo и raw fallback имеют ограничения | [SERVER_CONSTRAINTS.md](/home/svechnik/Projects/github_site/tomenet_interface/SERVER_CONSTRAINTS.md:43), строки 43–100 | Mock success нельзя превратить в authoritative state transition; ограничения связать со scenarios и behavior/protocol source |
| Item-use success/resources берутся из demo profile и generic fallback | [item-use.js](/home/svechnik/Projects/github_site/tomenet_interface/js/features/item-use.js:2), строки 2–38 | Вычисления fixture могут доказывать только presentation, никогда gameplay правила |
| Регистрация keyboard handlers централизована; router использует приоритет и первый consumed handler | [app.js](/home/svechnik/Projects/github_site/tomenet_interface/app.js:483), строки 483–526; [input-router.js](/home/svechnik/Projects/github_site/tomenet_interface/js/core/input-router.js:11), строки 11–35 | Evidence требует проверки реально вызываемого route в нужном context, а не поиска клавиши в исходнике |
| Скрипты явно подключены обычными script в значимом порядке; suite отсутствует | [index.html](/home/svechnik/Projects/github_site/tomenet_interface/index.html:1233), строки 1233–1289; [AGENTS.md](/home/svechnik/Projects/github_site/tomenet_interface/AGENTS.md:125), строки 125–132 | Нужно проверить reachability/init, page login отдельно, а не объявить каждый feature-файл активным; existing smoke evidence нельзя предполагать |

MENUS также явно отмечает missing Social/Knowledge/stores/settings/session/короткие диалоги ([MENUS.md](/home/svechnik/Projects/github_site/tomenet_interface/MENUS.md:492), строки 492–591). Это документированный gap inventory, не исчерпывающий машинный ledger. Guide module/локальный contextual help не доказывают полный Guide browser/search/bookmarks outcome: mapping задаётся actual entrypoint и scope, а не совпадением имени.

## Предлагаемая нормализованная схема

В HTML `docs/capabilities/coverage.json` и соседняя schema; native manifest/schema остаются в местах, выбранных решением Govern the cross-repo capability manifest. HTML snapshot хранится отдельно, byte-for-byte, с metadata digest; точные consumer filenames — соглашение реализации, структура ниже является предлагаемым контрактом.

Коллекции ledger:

- `coverage`: уникальная строка `{capabilityId, implementation, reason?, mappingIds, scenarioIds, approvalIds}`. `implementation` — только `missing|planned|prototype-complete`. Missing/planned требуют конкретного gap/reason; отсутствие реализации и частичный flow оба запрещают prototype-complete. `planned` означает явный план, а не автоматически любой `[ ]`.
- `mappings`: `{id, capabilityId, entityIds, sourceIds, entrypoints, scope}`. Entity IDs ссылаются на canonical surface/action/state/binding/context; entrypoint содержит page, module/factory/export или DOM selector, registration/init locator. SourceIds отдельно отмечают MENUS/HOTKEYS assertion и реальный код. Many-to-many допустимо; повторная кнопка не новая capability. Локальный experimental ID может быть отдельной коллекцией, никогда знаменателем shared coverage.
- `scenarios`: `{id, capabilityId, mappingIds, contextId, bindingIds, gates, fixtureIds, steps, expectations, applicability}`. Steps описывают precondition → entry → selection/prompt → confirm/cancel/retry → visual result/error → return/focus. Expectations имеют стабильные локальные IDs. Для states normal/empty/boundary/loading/unavailable/error, длинных строк/resize/minimum-scale/fallback и Reset/persistence перечислить obligations; каждая либо проверена, либо явно `not-applicable` с reason и source. Условный build/platform/server gate не позволяет молча удалить неудобный scenario.
- `evidence`: `{id, kind, scenarioId, expectationIds, sourceIds, artifact, artifactSha256, result, observedAt, observer, environment, manifestSha256, dependencyFingerprint}`. Kind — automated/manual visual acceptance или human approval record; result pass/fail. Environment фиксирует browser/page/file-or-http/viewport/keyset/platform и gate/fixture selection. Artifact с assertion outcomes обязателен; screenshot без сценария не достаточен. Fixture refs всегда UX/mock, evidence всегда acceptance. Evidence не создаёт behavior provenance.
- `approvals`: `{id, claim:"ux-approved", capabilityId, mappingIds, scope, evidenceIds, approver, decisionSourceId, approvedFingerprint}`. DecisionSource — реальный человеческий ответ/запись решения с revision/path/anchor/hash. Автор checker или generated checkbox не approver. Approval может быть дан на proposed layout даже при missing implementation; prototype-complete может существовать без approval. Scope approval не расширяется на новый atom или изменённый UX автоматически.
- `sources`: переиспользуемые `{id, role, repository, revision, path, anchor, sha256}` с четырьмя ролями решения manifest. Полный file digest — authority для drift, строки/символы — locator. Native behavior и protocol references разрешены только из нормативного native baseline/source; HTML docs/code/fixtures — ux, reports — acceptance.
- Header: `schemaVersion`, `consumer`, `manifestSha256`, `snapshot metadata`, `sourceInventory`, `evidenceEnvironmentVersion`. Ни статусов implementation, ни approval не добавлять в canonical manifest.

`sourceInventory` хранит отсортированный список path+SHA-256 всех scope files: README/MENUS/HOTKEYS/SERVER_CONSTRAINTS, index/login и их transitively loaded core/data/features/app/CSS/assets, relevant generators; включить также все files feature/data/core directories, чтобы замечать новый пока неподключённый module. Исключения должны быть явными и объяснёнными. Canonical bytes используются для snapshot/content SHA-256 (не Git commit, не prettified reserialization). Inventory fingerprint вычисляется из однозначно сериализованных sorted path+hash pairs. Evidence artifacts/ledger исключаются из собственной dependency inventory, чтобы избежать самоссылочного hash.

## Атомизация и evidence gate

Atomic unit — самостоятельно проверяемый outcome или различимый полный flow из binding/context/intent/result/cancel/error/focus/gates. Семейные LIFE/IN/... seed IDs baseline и input audit IDs не импортируются как canonical IDs без нормализации. Input audit уже различает keyset/macro/owner/exit ([single-window-input-loops.md](../../../docs/research/single-window-input-loops.md), строки 7–26): эти различия становятся contexts/bindings/scenario obligations, не отдельными capabilities для каждой клавиши.

Importer читает checklist rows как кандидаты mappings/declared coverage с точным locator и slice; затем человек/исследователь сверяет native owner и actual JS route. `[x]` не импортирует ux-approved и не импортирует prototype-complete без scenario evidence. `[ ]` → missing candidate; `[-]` → partial gap, effective coverage missing либо planned при явном плане. Несопоставленная строка или модуль требуют review; это не повод автоматически придумать игровую capability. Отдельные visual unavailable outcomes можно маппить без наследования полноты parent gameplay capability.

Prototype-complete допустим только при наличии всех требуемых flow expectations и актуальных pass evidence для применимых scenarios. Gate проверяет entrypoint, mouse/штатный keyboard route, child cancel с возвратом, success/error/unavailable, gameplay blocking/chat, focus, fallback/layout и persistence/Reset где применимо. В server-backed flow результат остаётся явно synthetic; сравнение с protocol fixtures может доказать adapter rendering, но не integration. Независимая проверка native parity относится к native consumer ledger и живому behavior/protocol acceptance, вне HTML claims.

## Checker/sync: предлагаемый алгоритм

1. Validate canonical manifest/schema и semantic invariants ровно по Govern the cross-repo capability manifest: ID uniqueness/type, relations endpoints, references, lifecycle/replacedBy/cycles, active behavior provenance.
2. Validate HTML snapshot schema, byte digest и source metadata. Если canonical local source доступен — сравнить bytes digest; если недоступен — freshness `unavailable`, никогда `current`/общий success. Существующий snapshot можно независимо проверить structurally, явно без freshness certification.
3. Sync только после успешной canonical validation: prepare snapshot+metadata+ledger во временных файлах, validate staged state, publish как согласованный набор с recovery/atomic replacement. При отказе сохранить прежний набор; не оставить новый snapshot со старым ledger.
4. Сохранить все прежние claims/history; новым active IDs добавить missing без evidence/approval. Deprecated/retired строки оставить как archive, исключить из active coverage. Split/merge/replacedBy не переносит complete или approval. Unknown/retired/experimental IDs не удовлетворяют shared coverage; unknown shared reference — ошибка.
5. Read-only check: schema плюс полное explicit active coverage; links/types/scopes/entrypoints/artifacts; допустимые claims; prototype-complete gate; человеческий provenance ux-approved; source role boundaries. Проверка может успешно подтвердить честный ledger с missing: это **ledger-valid**, не **prototype-ready**. Выдавать отдельно active denominator, missing/planned/complete, approved, stale/failed, gaps.
6. Recompute source inventory и evidence fingerprints. Добавление/удаление/rename/change любого relevant file отмечает drift; broken anchors/DOM routes — ошибка. Static match не доказывает flow; нужны acceptance outcomes. Initial безопасная политика — любой inventory/manifest digest change делает старые complete evidence и approval stale до явной повторной проверки/переподтверждения. Исторические claims остаются, effective claims не засчитываются. Более точная invalidation возможна только с доказанным dependency closure, а не списком «любимых» файлов.
7. Expected source/manifest mismatch, missing rows, failed/stale gate, broken sources, wrong roles, unresolved imported rows/modules — различимые diagnostics с именем outcome/source; machine report имеет стабильные codes и effective claims. Отдельный readiness mode требует current freshness и актуальных complete claims всех целевых capabilities; неизвестная freshness/неприменимый без reason не успех.

Команда не использует PR/merge/Git/network/submodule, не меняет canonical source и не переписывает UX docs автоматически. Инвентаризацию/линки можно проверять filesystem API; sync source задаётся аргументом/соседним repo согласно существующему решению. SHA capture незакоммиченного рабочего дерева обязателен; Git revision — дополнительная provenance.

## Проверка контракта и остаточная работа

Будущие checker acceptance cases: active ID без строки; duplicate/unknown ID; retired replacement; новый/удалённый module; unchanged HEAD+changed file; unchecked versus `[x]` импорт без approval; screenshot-message без file outcome; placeholder/ghost unavailable вместо execution; complete без cancel/focus/error evidence; modified fixture с прежним pass; canonical unavailable; stale approval; sync interruption; malformed relation endpoint; fixture mistaken for behavior. Это спецификация tests реализации, tests сейчас не созданы/не запущены.

Исследовательская неопределённость по устройству ledger закрыта. Осталось исполнение спецификации checker и наполнение атомарного manifest после остальных inventory. Полный native mapping не заявляется; человеческие approvals нельзя восстановить из checklist. Новых обязательных product решений исследование не обнаружило: если implementation захочет ослабить conservative invalidation или сократить acceptance scope, это уже отдельное явное изменение контракта, а не вывод о наличии parity.

## Fingerprints прочитанных ключевых источников (SHA-256)

```text
MENUS.md                         34289fc2933a032caacf8bc5fd42bc05fdfb6416ffde6a4bb862a71159ced310
HOTKEYS.md                       5c823c68eb77f7a0ae893f9589efe3ac3f2ab3ad49b1d7bc41453af5924c1674
SERVER_CONSTRAINTS.md             16b35475815007318a37710529178ed802729caf328317cd73d22a3af94cc76d
index.html                       5a6616906da9072561044afe251c32fc0f161002fa61b21b11b73fce63cbbc9c
app.js                           f513820e04b078ad8e85ed483d7ec864d7591f39070ddde5fe59e5de8bc1d421
js/core/input-router.js          8aa8cd0fc14ea202898a7cb70152807702e7c062689ebd2fd4d0fb75c62d906d
js/features/abilities-window.js  a9be971e76ef539135379bf346b2c2b96ed4862d600b9fc3979f14684601853e
js/features/ghost-powers.js      35ebde41e6eed2bc11c992dce5bfb53aafcbc2363657f4fd6e6a3db25bf7f02a
```

Это fingerprint выбранных исследовательских источников, а не готовый полный sourceInventory consumer.
