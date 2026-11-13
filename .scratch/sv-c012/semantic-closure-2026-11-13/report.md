# C012 — невизуальный semantic contract

Дата: 2026-11-13. Статус: **два технических вопроса разрешены без изменения canonical wording/IDs/allocation**; пользовательское отображение остаётся отложенной приёмкой. Проверены источники и существующий production-путь; runtime checks принадлежат отдельному core owner. Код, тесты и canonical документы этим исследованием не менялись.

## Итоговый контракт

| Вопрос | Невизуальный контракт C012 | Что этим не заявляется |
|---|---|---|
| Study count / availability | Сохранять полный полученный unsigned byte как числовое source value в модели и read-only view. Признак доступности для будущего consumer определяется `source_value != 0`, не заменяя исходное число. | Полученный byte не объявляется количеством skill points; bool не объявляется эквивалентом count. Итоговый видимый результат остаётся C012-DA-05 при C023. |
| Stat index / отдельное пользовательское значение | Сохранять каждый полученный current/max/index/base/temp с version defaults в read-only view. Source `stat.index` — индекс таблиц характеристик, отдельный от packet slot ID 0..5. Проверять slot ID до indexed update. | Нет требования печатать служебный индекс новой строкой Character или самостоятельно пересчитывать его вместо серверного. Поздние spell/stat-sheet consumers и их приёмка не получают PASS из хранения поля. |

Оба контракта уже выполняются текущим production-кодом. Изменение канонического описания не нужно для закрытия **технической части**: числовые source fields не потеряны и доступны для чтения. Историческое противоречие буквального user-facing `count` с реальным bool sender не скрывается и не превращается в утверждение «игрок видит количество». Теперь оно не требует выбора между потерей числа и потерей baseline: сохраняем число, а семантику availability задаём отдельно. Окончательная пользовательская приёмка назначена реестром отсрочек; полного outcome PASS здесь нет.

## Study: факты и границы

1. Canonical description — `Read available study count.`: `docs/capabilities/manifest.json:22839–22844`; точный snapshot result obligation — `docs/tasks/c012/coverage.json:526–545`.
2. Реальный server sender принимает `bool study` и посылает один byte: `src/server/nserver.c:7814–7823`. Обычный producer проверяет **ненулевые skill_points**, затем вызывает `Send_study(TRUE/FALSE)`: `src/server/xtra1.c:501–505`. Другой caller в debug slash также посылает TRUE: `src/server/slash.c:6305`. Источники не доказывают передачу полного skill_points count через PKT_STUDY.
3. Legacy принимает byte в `bool study`, вызывает `prt_study`: `src/client/nclient.c:3713–3723`. Вывод — наличие строки `Skill` при nonzero и очистка при zero: `src/client/c-xtra1.c:1287–1301`. Следовательно, `count` в названии capability нельзя использовать как основание выдумать недостающую серверную точность.
4. SV декодирует `h->study=byte(&c)` без нормализации в bool: `src/client/sv/protocol/status-update.c:97`; хранит в `int study`: `src/client/sv/session/status-state.h:39`; публикует выбранное поле: `src/client/sv/session/session.c:122`. HUD входит в копируемый app view: `src/client/sv/app.h:16`, `src/client/sv/app.c:613–614`.
5. Existing production-path fixture test проверяет значение **4**, не потерянное до1: `tests/sv/status.c:26–46`. Это проверка сохранности arbitrary byte, не доказательство реального server count. Derived presentation row также сохраняет число (`hud.c:122`), но не является доказательством видимого блока: отвергнутый StatusDetails не возвращается.

Риск: будущий skills consumer может ошибочно прочитать byte как skill_points или уничтожить исходное число приведением к bool. Правило выше предотвращает оба случая. При приёмке DA-05 проверять доступность по nonzero и отдельно достоверный источник actual skill points, если тот нужен C023; не выводить его из PKT_STUDY.

## Stat index: факты и границы

1. Canonical требует `Read each stat current/max/index/base/temp value with version defaults.`: `docs/capabilities/manifest.json:22758–22764`; точные obligations — `docs/tasks/c012/coverage.json:93–112`.
2. Сервер определяет `stat_ind` из эффективного stat value: 3..18 →0..15, диапазоны18/xx → таблица, верхний предел37: `src/server/xtra1.c:5214–5224`. Send_stat посылает этот index отдельно от stat slot: `src/server/nserver.c:6847–6877`.
3. Legacy принимает `s_ind` в отдельное поле `stat_ind[stat]`: `src/client/nclient.c:2133–2158`; определение `src/client/player.pre:430` прямо говорит `Indexes into stat tables`. Его настоящие consumers используют индекс для расчёта отображаемых spell failure outcomes: `src/client/c-spell.c:82`, `src/client/lua_bind.c:147–157`. Самостоятельный raw index label baseline этим не требует.
4. SV читает payload index, current/max/base и conditional temp в отдельный `SvHudStat`: `src/client/sv/protocol/status-update.c:65–72`; типы отдельно различают `SvHudStat.index` и `SvHudUpdate.stat_index` (slot): `src/client/sv/session/status-state.h:19,50`. Slot проверяется decoder и model до публикации: `status-update.c:71`, `session/session.c:107–114`. Payload index сохраняется как серверное source value, не используется для unsafe table access в этом C012 пути.
5. Existing production-path check подтверждает `current=108,max=118,index=20,base=110,temp=3`: `tests/sv/status.c:37–38`. Version-default case присутствует в `status.c:85–90`; invalid packet slot6 отвергается без публикации в `status.c:400–403`. Финальный run этих checks — отдельный core owner; source inspection не заменяет его.

Риск: путать packet slot ID и payload index либо выдавать сохранность индекса за выполненный поздний spell-failure consumer. В C012 достаточно чтения/сохранности каждого source field, без новой UI строки и без переноса поздних actions в текущий scope. При подключении таблиц будущий consumer обязан проверять допустимый table-index domain; C012 сейчас не делает такого lookup.

## Нормативные границы и следующий шаг

Применены `AGENTS.md`, корневой `CONTEXT.md`, `docs/agents/domain.md`, ADR0001 (сервер авторитетен, protocol/model/UI разделены) и ADR0004 (read-only revision-based views). Graphify query выполнен перед source research; его broad/truncated вывод уточнён прямыми исходниками. Противоречия с этими ADR не найдены.

Root/doc owner может записать эти два технических вопроса как **resolved — source values retained, baseline semantics documented**, сославшись на этот отчёт, и убрать их из невизуального blocking remainder. Не изменять canonical wording/statuses, не помечать весь Study/stats outcome PASS. Остаток видимого результата уже закреплён в `docs/tasks/c012/deferred-acceptance.md`: DA-05 для Study; DA-02/04 для stat-sheet/option-driven presentation. Прочие core/data/disposition пункты C012 этот отчёт не закрывает.
