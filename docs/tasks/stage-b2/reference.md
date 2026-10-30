# B2 — visual reference и карта экранов

Статус: reference frozen, 2026-10-28. Shell B2-001 реализован и визуально
сопоставлен на Linux ([evidence](../../sv-b2-001-evidence.md)); B2 принят пользователем 2026-10-30 ([решение](../../sv-stage-b2-acceptance.md)). Полное исследование и provenance:
[decision 01](../../../.scratch/sv-stage-b2-planning/issues/01-web-reference-and-screen-scope.md).
Общий контракт: [B2 spec](../../sv-stage-b2-spec.md).

## Источник и fingerprints

Read-only источник: `/home/svechnik/Projects/github_site/tomenet_interface`. На 2026-10-28 `git rev-parse HEAD` = `1bb2d5d45b9001bf2a6f2084f78184c29a73fca7`. Login HTML/CSS/data/background assets tracked и без tracked diff. Рабочее дерево всего web repo **не clean**: `M ../.gitigonre`, untracked `.playwright-cli/`, `.scratch/`, `AGENTS.md`, `HOTKEYS.md`, `MENUS.md`, `PLAYERS_ONLINE_DATA_CHECKLIST.md`, `README.md`, `SERVER_CONSTRAINTS.md`, `docs/`, `reference-full.png`, `world-reference.png`, `tools/update-login-news-snapshot.mjs`. Поэтому SHA HEAD недостаточно для воспроизведения документации. Ни один из этих файлов не изменён этим исследованием.

SHA-256 прочитанного reference:

| Relative path | SHA-256 |
| --- | --- |
| `login.html` | `038343aa9ee5aa8d1d2a8ca8fb5a87740730308a90ed7ea5dfd0b3cfa405e355` |
| `css/foundation.css` | `548a739c4be10bc9183feb11f91ba35a0b1585f25504669852cddf400c5ff368` |
| `css/theme.css` | `184d65e68b82a81eb26f9e1198efd0804c6ea96af250a74b8f1c765501ee3a06` |
| `js/data/creation-descriptions.js` | `e03790ba4d9a5ad16e855d58c1fd924f44a7a56a51e0077c65ba4df8da2b0fca` |
| `js/data/login-news-snapshot.js` | `fc7da1ee7b64f5d718b98f658518ececfc85876951a9665cd1123f342f8d63fe` |
| `README.md` | `3a491afa9fa76580994c6f0c88ba6064679442a6b22e10617311f3bf6e0b5886` |
| `SERVER_CONSTRAINTS.md` | `16b35475815007318a37710529178ed802729caf328317cd73d22a3af94cc76d` |

Backgrounds ниже — ровно 19 PNG, перечисленные в `login.html:2178–2196` (не все произвольные файлы assets). SHA-256 фиксирует каждый фактически читаемый файл:

| Asset under `assets/log_in_backgrounds/` | SHA-256 |
| --- | --- |
| `cirith_ungol_ascii_seed06.png` | `39c853c65294fe9907c6643386dafe49931322dc91095f133c3a7878bf500296` |
| `cloud_planes_ascii_seed14.png` | `8b4ac7df7c53cd17eeddeff78322322d560ae204f3572d463ddba2104e3c2eff` |
| `erebor_ascii_seed19.png` | `2a1d09395720428e78d28978a1d0f5b9c47862133d88d578d24173727da3fed9` |
| `halls_of_mandos_ascii_seed9.png` | `3f084173e26ee05ff246bf6f0b643697ecaf7e5ac1d8a8bdc29f3a80042f9795` |
| `heart_of_earth_ascii_seed17.png` | `21e5f55fdc370833b7683fbff208e61701edfa62a2f33f779143807a8e845f39` |
| `helcaraxe_ascii_seed31.png` | `66b9661b90ab06e0a4a816df5f12d220ca8afa92829e054761af936947873298` |
| `illusory_castle_ascii_seed16.png` | `2725e1b628b2f8098271db59e040c7b1439ef59249f5eed91e83535d54750ab1` |
| `land_of_rhun_ascii_seed36.png` | `7b777f13396e846f1e83056ba250728d4e9b3d2b876a05c444d51cd8cf09ff4a` |
| `mirkwood_ascii_seed01.png` | `91930ad54afe333aaaa4ab2e107f5e6697b819d924d25ec5d4ca97236031bcd7` |
| `moria_ascii_seed22.png` | `693885b12737466a502bd3f13cc80f5621d55411f588159f8b9321cb41fe2c53` |
| `mount_doom_ascii_seed24.png` | `e4324e912bd0e24edff8adfdae3b696d0ecfd85ee97354497d730a1d95820f2a` |
| `nether_realm_depth195_ascii_seed34.png` | `67856eb59b868b3c28c99a562ae256a58b938934522053dde03af5354088f919` |
| `old_forest_ascii_seed04.png` | `f4f6a31a22cadf8cfe3f7bd4cd07ed28e9682ef6ed0414da4b660a1e2a7ace95` |
| `orc_caves_ascii_3168x1584.png` | `3a404fe9b93854bc3019689c6746f12a3f7faa20c76c536146f2def904a28a96` |
| `sacred_land_of_mountains_ascii_seed70.png` | `3a8f351174d8df9d8c2b81a0bb2cd34626a9c246ca663c80441f0c6d4b5a3357` |
| `sandworm_lair_ascii_seed31.png` | `549ca59150fc9fddee81f27b2e0005a5996ce7b95941f3afe7dcb8a9f6133dbc` |
| `small_water_cave_ascii_seed44.png` | `9a36cd20ced8e9ef19eb2fd59590a50fa37c2a56b18b55287c59dc5a3ccf16c6` |
| `submerged_ruins_ascii_seed01.png` | `49ee55de6c97623b5df0c0073969f2e3bb9e5fa27a15f6eaac12c9ba889093ec` |
| `training_tower_ascii_seed404.png` | `c60bfec30473fc0bf9f01da8bd3ce929d74a21d127bc5929076fd8df96ec4b57` |


До начала реализации B2-001 сохраняет воспроизводимый snapshot HTML/CSS/data
и используемых ресурсов либо их доступный локальный source capture с exact
hashes. Native resources могут одновременно служить snapshot assets, без
дублирования тяжёлого набора изображений. Внешний mutable path сам по себе не
является завершённым reference freeze. Изменение upstream требует явной сверки
и обновления выбранного reference; новый HEAD не подставляется автоматически.
Выбранные background/animation frame и Linux font фиксируются при visual capture.

## Композиция и ресурсы

Источник — **итоговый CSS cascade D**, не промежуточные старые варианты в том же
HTML. Body override 1104–1110 задаёт monospace Consolas/Liberation Mono/DejaVu
Sans Mono, цвет `#eeeadd`, фон `#020403`; шрифтовые роли остальных компонентов
снимаются с итоговых styles. Ранний sans body stack и старое README описание
birth detail справа не являются reference.

- Wide: shell до 1720 logical units, внешний отступ 15; равные side columns
  180–340, центральная panel clamp(400, available width−414, 860); большой
  клеточный ASCII logo сверху, Chronicle слева и realm links справа.
- При width ≤1240 final override перестраивает hero/panel/links в column,
  скрывает Chronicle/torches/link labels; шесть icons располагаются под panel.
- При height ≤800 и width >820 уменьшаются hero/inter-item интервалы,
  server card height 72 вместо 88. Text не shrink-to-fit; формы/списки scroll.
- Character overview: compact inline rows/slot placeholders; `.character-detail`
  скрыт итоговым CSS (`login.html:1070`). Отдельная detail side-panel не добавляется.
- Birth: summary слева, selection сверху, description снизу с 60/40 split;
  самостоятельный scroll. Сохраняются обе native class/race последовательности.
  Birth `.creation-detail` отображается; отдельного step-list/progress widget нет.
- Переносятся black/green/gold palette, translucent panels/borders, fields,
  buttons, selected/focus/hover/disabled/error/loading states и overlays.
- 19 ASCII background PNG, random initial выбор, rotation около 30 s, fade 2 s,
  gentle movement 9 s; animated cell logo и torches около 100 ms. Native UI
  использует неблокирующий elapsed time; hidden/minimized останавливает
  декоративную работу, reduced-motion даёт static frame. Эти элементы входят
  в перенос дизайна, а не считаются необязательными просто из-за декоративности.

Текущий native minimum — 1024×768 logical. Mobile CSS ≤820/540 не расширяет
B2 scope. Reference screenshots сравниваются в том же supported logical размере.

## Screen/state → implementation owner

| Surface / state | Web reference | B2 owner / native binding |
|---|---|---|
| Общий shell, branding, backgrounds, logo, torches, Chronicle/links | `body[data-variant="D"]`, `.login-background`, `.login-shell`, `.login-hero`, `#accountLogo`, `.account-torches`, `.realm-links` | [001](SV-B2-001-shell.md); один SDL window, ресурсы и final cascade |
| Server list, selected/empty/pending/unavailable | `#serverStep`, `#serverGroups`, `.server-option`, `.server-enter`, `.server-health` | [002](SV-B2-002-server-account.md); existing endpoint selection и native telemetry |
| Direct connection edit/error/cancel | `#directBackdrop`, `.direct-backdrop`, `.manual-card` | 002; существующий host/port editor и baseline accept/cancel |
| Account/password, auth/create-account, validation/rejection/provider status | `#accountStep`, `.account-form`, `#accountInput`, `#passwordInput`, `#passwordToggle`, `.field-error` | 002; existing credentials interaction, real provider status, masked field/private reveal |
| Connecting/auth waiting, failure/retry | `.loading-mask`, `.loading-card`, `#loadingTitle`, `#statusLine` | 002 для contact/auth; [005](SV-B2-005-motd-states.md) для общего wait/error компонента |
| Character overview, inline fields, selected/empty/full/disabled | `#characterStep`, `.character-list`, `.character-option`, `.character-level`, `.mode-badge`, `.character-location` | [003](SV-B2-003-character.md); native rows/server slot IDs, никакого mock roster |
| Ordinary/exclusive slot, editable name, rejection | `#createCharacterButton`, `#createExclusiveButton`, `#characterNameInput` и creation visual family | 003; existing native actions/availability/draft/recovery |
| Birth choices/stats/mode, summary, legal/disabled, back/cancel/confirm | `#characterCreateStep`, `#creationSummary`, `#creationStageTitle`, `#creationContent`, `.creation-choice-grid`, `.creation-detail`, `.creation-stat` | [004](SV-B2-004-birth.md); native legal/highlighted IDs и interaction state |
| Birth child confirmation/Guide/quit | `#attributeWarningBackdrop`, `#continueWithUnspentButton`, `#creationGuideBackdrop`, `#quitStep` | 004/005; actual caller-specific actions и exact temporary Guide text |
| Final ACK/partial/error, MOTD, handoff | Native-specific adaptation + отдельный MOTD prototype C | 005; authoritative ACK gate, any-key/skip/once-only, disabled gameplay |

Owner 005 определяет общие overlay/wait/error стили; остальные owners
подключают их к своим production состояниям. Нет второго semantic owner.

## Native adaptations и границы mock content

| Reference content/control | Решение B2 |
|---|---|
| Demo servers, population/ping, account replies, roster, birth numeric previews | Только настоящие native/session данные; нет фиксированных demo ответов или принятия JS правил как protocol model. Неизвестные данные получают честный unavailable/empty state. |
| World Chronicle | По уточнению пользователя от 2026-10-29 получение данных из сетевых источников реализуется сразу в текущей корректировке B2. Offline snapshot не является итоговой реализацией; оформление событий следует reference. |
| Six realm links/IRC | Сохранить расположение/icons/labels; по уточнению пользователя от 2026-10-29 форма иконок строго круглая, без приплюснутой нижней части frozen CSS. Если production launcher уже доступен, использовать его политику; иначе явный unavailable control/adaptation, без реализации OS integration C060 или Guide C062. Browser prototype navigation не переносится. |
| Reorder S/I/A | Не реализуется: owner C039. Любой элемент макета, подразумевающий reorder, явно unavailable/disabled либо документированно исключён без искажения layout; не возвращает исторический широкий B. |
| DNA/random/automatic-DNA | Существующие B interaction commands и их доступность сохраняются; новая DNA persistence C040 не добавляется. Guide остаётся exact placeholder. |
| Secure storage | Сохраняются существующие B store/lookup/opt-in/status и session-only policy. External replace/delete в vault tests не означает новые пользовательские delete/replace actions. |
| Password reveal | Допустимое local UI state по `#passwordToggle`; private render-only доступ при явном reveal, сброс при уходе. Нет raw secret в retained/public views, diagnostics, logs или captures; capture делается masked. |
| Descriptions/illustrations | Локальные visual resources допустимы с provenance. Не выдумывать server-authoritative характеристики; если native source не даёт значения, не подставлять mock число. |
| Native-only skip/errors/storage/availability | Компонуются теми же fields/status/modal компонентами; сохраняют конкретный caller contract. Отсутствие готового HTML кадра не разрешает оставить старый текстовый экран. |

## MOTD — отдельный утверждённый reference

[Decision 28, вариант C](../../../.scratch/single-window-sdl3-client/issues/28-prototype-the-server-defined-motd-screen.md#answer),
`.scratch/single-window-sdl3-client/motd-prototype.html?variant=C`;
capture commit `e271c582cbf89d404deda6189881e531d3775770`.
Сохраняются все 23 server-defined строки, исходные visible columns/colors/ASCII
и неизвестное содержимое. Centered maximum uniform fit, padding 24 logical units,
без wrap/scroll/crop или клиентских welcome/headings. Оформление D не накладывается
поверх server composition. Ordinary login HTML не содержит MOTD.
