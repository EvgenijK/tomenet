# План функциональных этапов SV после B

2026-10-06. Этапы A (8 outcomes) и B (54 outcomes) заморожены. Новое
разбиение C001–C064 применяется к 863 outcomes прежних C–G; полный denominator
остаётся равен 925. Machine-readable authority —
[`capabilities/stages.json`](capabilities/stages.json), schema —
[`capabilities/stages.schema.json`](capabilities/stages.schema.json).

Каждый этап после B содержит только один namespace `capability.<block>`. Один
блок может иметь несколько последовательных slices: ранний slice содержит только
минимум для usable client, поздний принимает optional или интеграционное поведение.
Это acceptance slices, а не требование создавать отдельный архитектурный модуль.

## Последовательность

| Этап | Блок | Outcomes | Scope |
|---:|---|---:|---|
| A | `baseline` | 8 | Frozen synthetic foundation |
| B | `baseline` | 54 | Frozen pregame baseline |
| C001 | `platform` | 1 | Запуск установленного клиента |
| C002 | `settings` | 6 | Bootstrap settings data без UI |
| C003 | `files` | 10 | Core resources и persistence |
| C004 | `preferences` | 3 | Preference loading без macro precedence |
| C005 | `input` | 3 | Keymap, quantity, raw key |
| C006 | `fonts` | 11 | Text/map fonts и fallback |
| C007 | `rendering` | 15 | Визуальный игровой rendering |
| C008 | `network` | 6 | Оставшиеся network controls |
| C009 | `lua` | 1 | Startup Lua reload |
| C010 | `transfer` | 5 | Server file transfer |
| C011 | `session` | 5 | Session preparation |
| C012 | `status` | 27 | Live HUD/status |
| C013 | `world` | 1 | Основная live-карта (`world.read-map`) |
| C014 | `session` | 4 | Live session lifecycle |
| C015 | `map` | 7 | Overview/locate карта |
| C016 | `messages` | 9 | Live и historical messages |
| C017 | `request` | 9 | Server requests |
| C018 | `target` | 8 | Targeting и inspection |
| C019 | `direction` | 6 | Direction selection |
| C020 | `world` | 19 | World movement и interaction |
| C021 | `items` | 23 | Inventory data и item-selection surfaces |
| C022 | `items` | 48 | Ordinary item actions |
| C023 | `skills` | 6 | Skills и abilities |
| C024 | `spells` | 34 | Spells и special abilities |
| C025 | `items` | 5 | Spell-dependent item actions |
| C026 | `combat` | 11 | Combat actions |
| C027 | `store` | 28 | Ordinary stores без `store.service` |
| C028 | `chat` | 6 | Chat |
| C029 | `exports` | 2 | Data export |
| C030 | `information` | 43 | Information surfaces |
| C031 | `documents` | 21 | Documents |
| C032 | `session` | 8 | End-of-session flows |
| C033 | `housing` | 11 | Housing |
| C034 | `special-store` | 12 | Special-store presentation |
| C035 | `server-flow` | 15 | Server-driven gameplay flows |
| C036 | `store` | 1 | `store.service` |
| C037 | `credentials` | 1 | Credential update |
| C038 | `account` | 4 | Account management |
| C039 | `character` | 5 | Character ordering |
| C040 | `birth` | 2 | Birth DNA persistence |
| C041 | `admin` | 46 | Administration |
| C042 | `clipboard` | 4 | Clipboard |
| C043 | `social` | 23 | Party, guild и hostility |
| C044 | `settings` | 22 | Settings interface |
| C045 | `options` | 32 | Input, messages и alert options |
| C046 | `options` | 35 | HUD, lighting и visibility options |
| C047 | `options` | 44 | Map identity, animation, movement и targeting options |
| C048 | `options` | 41 | Item handling и session options |
| C049 | `options` | 32 | Audio и client-integration options |
| C050 | `alerts` | 5 | Game alerts |
| C051 | `imports` | 8 | Legacy import |
| C052 | `preferences` | 1 | Macro preference precedence |
| C053 | `input` | 3 | Macro input runtime |
| C054 | `macros` | 35 | Macro management |
| C055 | `audio` | 25 | Audio |
| C056 | `configuration` | 34 | Editable client configuration |
| C057 | `platform` | 5 | Optional platform capabilities |
| C058 | `lua` | 1 | Local Lua execution |
| C059 | `files` | 1 | Shared INS files |
| C060 | `os` | 2 | OS integration |
| C061 | `screenshots` | 3 | Screenshots |
| C062 | `guide` | 14 | Guide core без bookmarks |
| C063 | `files` | 2 | Guide bookmark persistence |
| C064 | `guide` | 3 | Guide bookmark actions |
| **C001–C064** | **46 namespaces** | **863** | Все outcomes прежних C–G |

## Правила slices

1. Prerequisite другого блока принимается раньше, а не включается скрыто в scope.
2. Повтор namespace продолжает production-путь предыдущего slice.
3. Перенос меняет порядок acceptance, но не behavior description, implementation,
   evidence status или evidence obligations.
4. Сумма C001–C064 обязана быть 863, каждый прежний C–G outcome назначается один
   раз. Каждый Cnnn владеет не более чем 50 outcomes; замороженный B54 является
   явным исключением. A/B rows сохраняются semantic deep-equal.
5. `stageCatalogSha256` ledger связывает schema v3 с exact bytes каталога.

## Late-block policy

После core gameplay намеренно расположены social/party, UI настроек, game options,
alerts, import, macro runtime/management, audio, editable configuration, optional
platform/Lua/INS, OS integration, screenshots и Guide. Ранние settings, files и
preferences только загружают необходимые данные. Визуальная погода относится к
rendering, звуковая — к позднему audio.

## Prerequisite metadata corrections

При reallocation удаляются только hard-dependency edges, которые ошибочно делают
optional invocation/integration prerequisite самого пользовательского outcome:

- из `session.load-profile-input` удаляются `input.macro-*`, `macros.load`,
  `preferences.macro-precedence` и `items.autoinscribe-on-update`; обязательные
  profile/settings/input data остаются;
- `input.macro-match` удаляется из ordinary action outcomes, но сохраняется у
  `macros.*` и внутри C053; `macro-wait/xwait` явно требуют `macro-match`, тогда
  как macro invocation обычного action является альтернативным binding;
- `rendering.weather` больше не зависит от `audio.weather`: оба являются
  независимыми consumers semantic weather state;
- `world.walk/run/open` не зависят от downstream store surface, а
  `world.stay/stay-one` — от downstream pickup interaction;
- `store.service` не зависит от `screenshots.capture`;
- `chat.send/history` не зависят от optional `clipboard.paste`;
- core font select/filter outcomes не зависят от поздней settings transaction UI;
- credential update C037 предшествует account password management C038.

Полный детерминированный список удалённых edges печатает
[`tools/reallocate_functional_stages.py`](../tools/reallocate_functional_stages.py).
Script также проверяет counts каталога, один block на post-B stage, A/B equality и
общие суммы. Stage-order validator дополнительно проверяет, что каждый оставшийся
prerequisite находится в том же или более раннем этапе.

`capability.map.*` означает overview/locate. Основная игровая карта —
`capability.world.read-map`, поэтому она отдельно принята в C013.
