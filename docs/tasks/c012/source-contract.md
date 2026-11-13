# C012 — source contract и production seams

Статус: planning analysis, 2026-11-11. Это анализ исходников, не evidence
реализации или приёмки. Canonical allocation и статусы registry не изменены.

## Уже существующая основа

- `src/client/sv/protocol/protocol.c:171`: явный список поддержанных packets;
  из собственно HUD сейчас реализованы HP и accepted character identity.
  `sv_protocol_next` (`:189`) выдаёт decoded `SvChange`, а не команды renderer.
- `src/client/sv/protocol/hp-update.h:12`: production HP decoder с version branch,
  bar marker, boosted marker и drain; его нужно сохранить и расширить вокруг
  него HUD, не писать второй HP decoder только для нового экрана или теста.
- `src/client/sv/session/session.h:35`, `session.c:62`: `SvStatus` пока представляет
  только HP. `SvSession` владеет состоянием; repeated complete HP обновления
  увеличивают revision. Полный HUD нельзя объявлять реализованным по этому срезу.
- `src/client/sv/session/session.c:121`: authoritative identity применяется
  с проверкой полученного от сервера character setup; повторная загрузка профиля
  из renderer не нужна. Accepted name/identity и необходимые descriptions уже
  доступны через existing session/app API.
- `src/client/sv/app.c:545`: bounded decode → apply → effects → publish loop;
  `:609` обновляет status/messages/ping/identity и вызывает alerts. Новые HUD
  changes должны использовать этот production путь и сохранить порядок
  однократных реакций между complete inputs.
- `src/client/sv/ui/status.c:11`: derived cache отдельно учитывает generation,
  semantic revision и geometry/font/scale key. Это HP-only cache, не полноценный
  HUD renderer. Переработка должна сохранить ADR-0004 и ownership копий.
- `src/client/sv/session/alerts.[ch]`: существующая HP reaction boundary;
  расширение представления не должно повторять sounds/notices на каждом draw.

## Authoritative baseline handlers

Источники wire/layout/field semantics находятся в `src/client/nclient.c`;
видимые состояния, пороги и precedence — в `src/client/c-xtra1.c`.
Legacy `Term`, screen coordinates и `screen_icky` не являются моделью SV.

| Группа | Основные production sources |
|---|---|
| HP, mana, stamina, sanity | `nclient.c:2170`, `:3109`, `:2232`, `:2084`; `c-xtra1.c:527`, `:842`, `:675`, `:1017` |
| Stats, armour, combat bonuses | `nclient.c:2133`, `:2263`, `:2935`; `c-xtra1.c:136`, `:500` |
| Experience/levels, carried gold и bank | `nclient.c:2959`, `:3084`; `c-xtra1.c:197`, `:462` |
| Title, demographics/body, location/depth | `nclient.c:3589`, `:2907`, `:3624`; `c-xtra1.c:167`, `:1049` |
| State, confused, poisoned/diseased, study, blows/wraith, food, fear, speed, cut, blind/hallu, stun | `nclient.c:3576` и `:3684–3830`; соответствующие `prt_*` в `c-xtra1.c:1093–1322`, `:1638–1692` |
| Tracked monster health | `nclient.c:6217`, `c-xtra1.c:1882` |
| AFK, encumberment, extra status, idle, indicators | `nclient.c:6301`, `:6321`, `:6373`, `:6648`, `:7357`; `c-xtra1.c:1704`, `:1724`, `:1753`, `:1323` |

Точное распределение 27 canonical outcomes берётся из allocation/manifest;
эта таблица группирует исходники и не создаёт новых outcomes. В частности,
`PKT_TARGET_INFO` (`nclient.c:5515`) имеет собственные message/history/cursor
эффекты; его нельзя автоматически отнести целиком к C012 из-за слова target.

## Версионные и семантические ловушки

- HP drain присутствует строго после `4.7.0.2.0.1`. Boosted HP распознаётся по
  `current > 5000`, а не `>10000`: отрицательные HP после смерти под boost
  должны сохраниться. Effective bar также учитывает client option.
- Mana/stamina bar marker кодируется в maximum (`>10000`); mana `-9999`
  имеет особый смысл для low-mana warning. Не нормализовать все отрицательные
  значения к нулю и не заменять unknown/absent общим фиктивным числом.
- Sanity: label/attr всегда в supported branch; damage после
  `4.6.1.2.0.0`, current/max начиная с `4.8.1.3.0.0`; older defaults — 0/0.
  Сохранить `SHOW_SANITY` contract и отдельные semantics numeric availability.
- Stats: temp field с `4.7.4.6.0.0`, старый boost marker — bit `0x10` в stat ID.
  После извлечения marker требуется безопасная проверка индекса stat.
- Armour: с `4.7.3.1.0.0` unknown `>15000` вычитается на 20000 перед boost
  `>5000`/10000; в `4.7.3.0.0.0` прежний bit `0x1000`. Base и plus хранятся
  отдельно; единственное итоговое число теряет данные для других consumers.
- Experience: previous threshold отсутствует до/в `4.5.6.0.0.1` и получает
  значение next threshold; level `>=1000` включает fractional-exp marker.
- Depth: четыре wire layouts и дополнительные старые colour semantics;
  location names/preposition defaults не угадываются по современному packet.
  `c-xtra1.c:1085` извлекает no-tele из sector colour; `:1272` учитывает его
  при speed presentation. Это межполевая зависимость status, не SDL focus.
- State различает paralyzed, stasis, suspended с приоритетом над search/rest;
  poison и blind/hallu — encoded states, не простые bool.
- Speed boost bit `0x100` применяется только при неотрицательном speed;
  не очистить этот bit безусловно у отрицательных чисел.
- Blows: label добавлен с `4.9.1.0.0.1`; absent label должен быть явно пустым.
  Не переносить потенциальное чтение uninitialized legacy buffer в SV.
- Encumberment имеет 13/14/16-byte layouts; absent rogue/heavy-swim/heavy-tool
  получают нулевые defaults. Приоритеты simultaneous flags видимы в
  `c-xtra1.c:1724`; хранить каждый flag, а не только один итоговый значок.
- Indicators: сохранить полный `u32` включая unknown bits, decoded meanings и
  compile-time `IND_WRAITH_PROB` presentation branch.
- Extra status и server labels остаются bounded исходными bytes с colour
  controls; renderer строит безопасную projection без переписывания source.
- AFK — server state, idle — отдельный server state. Idle baseline включает
  audio reactions; demographics body change включает macro-load reaction.
  Эти данные и однократные integration effects нельзя приравнять к draw.

## Preparation, applied/deferred и live adoption

`src/client/sv/login/preparation.h:56–89` уже предоставляет retained `SvApp`,
transport, FILE continuation и ordered drains. C014 принимает существующий
owner и его partially decoded protocol state; создание второго app/decoder
теряет авторитетное состояние и может повторить effects.

В `preparation.c:282` каждый packet проверяется по
`sv_protocol_supports_packet`. Поддержанные packets подаются существующему app
и применяются сейчас. Остальные known startup frames только framing-decode и
FIFO retention (`:292`); например `PKT_INDICATORS` уже поддержан startup-prefix
framer (`protocol/startup-prefix.c:35`, `:147`), но пока не semantic decoder.

Добавление новых C012 packets в supports автоматически меняет preparation
маршрут: они начнут обновлять retained HUD до live handoff. Должны сохраняться:

1. Complete frame только один раз изменяет модель и запускает реакции.
   Applied occurrence нельзя replay после adoption; sequence gaps intentional.
2. Deferred frame после byte drain принадлежит caller; его drain сам по себе
   не применяет change и не делает его applied. Проверить
   `sv_session_preparation_was_applied` и `take_deferred` contract.
3. Все deferred frames выдаются FIFO до raw tail; частичный app packet остаётся
   в том же app, не копируется в raw tail.
4. Порядок между ещё deferred changes и уже applied status важен при future
   cross-domain reactions: перед обработкой live tail определить явную
   стратегию replay/application, не откатывать уже новый status старым frame.
   Это integration concern C013/C014, не разрешение silently drop frames.
5. Draw/read-only view ничего не ack/reload/replay. Stale generations не меняют
   новый HUD; reconnect очищает session-derived availability и caches.

## Рекомендуемые границы реализации и проверок

Группировать работу вокруг status model и семейств baseline semantics:
resources; progression/stats/combat; identity/location/body; conditions и
indicators; native projection и компактная evidence-сводка. Decoder/app/model
результаты каждой группы должны быть production, доступными следующему owner,
а не test-only fixtures. Полный scope/IDs закрепляются отдельными tickets.

Linux — обязательная платформа; Windows/MinGW/Wine deferred. Только необходимый
минимум core проверок production пути: focused version/default/split+adjacent
inputs, coherent status projection, reset/stale lifecycle. Existing HP checks
не принимают полный live HUD. Broad harness, visual matrix и cumulative suites
не назначаются автоматически. Late actual socket lifecycle относится к C014,
map ingress — C013; core result не означает их PASS. Native C012 projection
должна быть подключена к production модели, а late combined evidence честно
остаётся pending до проверки.

Блокирующих вопросов к пользователю исходники не выявили. Точные C interfaces
и grouping — решения реализации в пределах ADR и isolation rule. Если при
реализации потребуется менять baseline/UX или расширять проверки, это отдельное
решение пользователя. Возможные исправления legacy (включая absent old blows
label и startup-only various demographics delivery) не входят в C012 сами по
себе; при подтверждении отдельного улучшения оформить его в sv-improvements.


## Согласованный план и UX reference

Итоговая группировка закреплена в [README](README.md): foundation 001; vitals 002 (4 outcomes), character values 003 (6), conditions 004 (10), extended status 005 (7); native caller 006 и evidence 007. Таблицы выше включают контекстные source handlers title/state/depth, а не дополнительные owned IDs. Минимальные ancillary semantic inputs, необходимые для owned status presentation (например no-tele для speed), не принимают целиком map/location capabilities.

HTML gameplay reference: `/home/svechnik/Projects/github_site/tomenet_interface/index.html`, `css/hud.css`; точные selectors и ограничения — [006](SV-C012-006-native-hud.md). При реализации зафиксировать актуальные SHA. `app.js` демонстрирует synthetic behavior; baseline semantics берутся из legacy.

Финальный независимый review подтвердил exact canonical snapshot, единственное владение 27 IDs и обязательный собственный native caller до полного C014 lifecycle. Отдельные возможности улучшения документации/legacy внесены в [sv-improvements](../../sv-improvements.md), не в scope реализации.


## Повторный критический review — 2026-11-11

По запросу пользователя три агента повторно проверили scope, production feasibility и acceptance. Семь задач сохранены: новых ticket IDs не требуется, canonical 27 outcomes / 81 obligations и их единственное ownership не изменены. Уточнения внесены непосредственно в задачи:

- 001/002: HP occurrence отделён от общей HUD revision; обновление mana/AFK не повторяет предупреждения, одинаковые HP packets сохраняют baseline reactions. Существующие HP effects не отключаются.
- 001/003/005: значимые body/idle transitions имеют occurrence-aware generation/sequence и явный delivery/ack либо bounded retention/overflow; latest state не заменяет промежуточные реакции.
- 006: production HUD renderer находится в src/client/sv/ui, existing diagnostic caller лишь вызывает его через production packet/app путь. Identity boundary C011 не расширяется в live loop.
- 006: status-only read subset существующего Character primary доступен через left shortcut даже в small layout. Full C030 не принимается; добавлены динамические HTML reference files.
- 004/007: минимальный production DEPTH consumer обеспечивает speed/no-tele и обновление projection при обоих порядках DEPTH/SPEED, без расширения приёмки до карты C013.
- 002–005/007: минимальный core набор проверяет semantic interpretation branches наряду с wire layouts. Новая suite или broad matrix не назначается.

Блокирующих вопросов пользователю не осталось: доступ в small layout обеспечивается существующим утверждённым Character primary entry, новое UX решение не требуется. Если конкретное обязательное поле нельзя представить в утверждённой surface, узкий вопрос возникает до зависимой реализации. В этом проходе изменены только planning документы, runtime tests и реализация не запускались.


## Реализованный контракт — 2026-11-11

`session/status-state.h` содержит typed SvHud/updates и bounded status occurrences; `protocol/status-update.[ch]` декодирует complete version-aware status/DEPTH packets. `session/session.c` применяет изменения атомарно и сохраняет HP revision независимо от HUD revision. `app.[ch]` предоставляет copied HUD view и generation-aware consuming effect API.

`ui/hud.[ch]` — production semantic preparation/rendering/input; existing diagnostic caller подаёт raw fixtures через тот же app. Character status-only primary доступен в small layout. Повторный Shift+C — idempotent open, последующий TEXT_INPUT не уходит в game input. Native read/draw не consumes effects.

Оркестратор подтвердил Linux build, focused status runner и native small после final fixes; один scoped C011 preparation actual socket case PASS. Backend/protocol/model sources и preparation test не менялись после того проверенного run; соответствие hashes проверено. Подробное evidence и pending late combined flows: [отчёт](../../sv-c012-evidence.md). Пользовательская приёмка не заявлена.
