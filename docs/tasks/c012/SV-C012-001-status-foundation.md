# SV-C012-001 — модель статусов и production decode contract

Статус: **accepted — Linux status/core**, 2026-11-13, по решению пользователя.

[Решение о приёмке](../../sv-stage-c012-acceptance.md). Интерфейсные и поздние интеграционные приёмки остаются отложенными.

## Закрытие технической части — 2026-11-13

**Невизуальные обязательства текущей реализации C012 закрыты.** Финальная Linux сборка и existing focused production status check прошли на stable existing-slots freeze3; Study/stat-index contract уточнён без canonical изменений, данные и поддержанные event flows проверены. [Итог и точный disposition27/81](final-disposition.md) разделяет core evidence, latest-state fields, occurrence retention и отложенные приёмки.

Интерфейс, small/resize и поздние consumers остаются в реестре отсрочек; они не получают PASS и не блокируют завершение технической части. Полная пользовательская приёмка, canonical registration и actual live lifecycle не заявлены. Новых блоков Character, кода, тестов и suites этой волной не добавлено. Исторические открытые статусы ниже относятся к предыдущим snapshots и перекрываются этим итогом.

Исторический статус до финальной проверки: **implemented, Linux core checked**, 2026-11-11; пользовательская приёмка pending.

Labels: enhancement

Cross-cutting основа 27 outcomes; отдельные capability IDs не добавляются.

## Работа и критерий готовности

1. Расширить существующий SV session/presentation owner и protocol путь для typed status changes. Существующие HP/identity API изучить и использовать в необходимых границах; не строить второй test-only state owner. Coarse status revisions допустимы, публичный view не раскрывает packet/Term/globals.
2. Decoder публикует только полный packet согласованной версии, обеспечивает точное потребление bytes и distinct waiting/error. Защитить индексы/строки по baseline limits. Поля без wire representation получают baseline defaults, unknown/opaque data сохраняются.
3. Main-thread owner применяет update атомарно, хранит availability, generation и revisions; reset удаляет прежние значения и кэши, stale input не меняет новую сессию. Derived UI cache учитывает font/scale/geometry наряду с revisions.
4. Согласовать единый status ingress contract с C011: preparation уже использует retained SvApp для supported packets. Applied occurrences не replay; deferred exact frames идут FIFO перед raw tail. Расширение supports_packet меняет dispatch preparation, поэтому исключить двойное применение и перестановку относительно deferred frames. Если для порядка нужен ограниченный SV-local seam, он входит в эту работу; полный live loop не нужен.
5. Не заменять известный, но ещё не реализованный gameplay packet успешным no-op либо recovery unknown. Явно передать неподдержанный caller/lifecycle owner согласно существующему bounded contract.

Минимальная проверка через production protocol/app/session: complete против partial packet, revision/read-only inspection, interleaved updates, reset/stale generation и once-only applied/deferred boundary. Использовать один необходимый focused core набор совместно с 007.

## Уточнения production contract после review

- **HP occurrence отдельно от общей revision HUD.** Сейчас `src/client/sv/app.c:616` вызывает `sv_alerts_evaluate` для каждого change, а `session/alerts.c:4` различает события по HP revision. Расширение status не должно порождать low-HP warnings на mana/AFK/resize. Сохранить discriminator HP occurrence/region: два полных одинаковых HP packet остаются двумя baseline occurrences, read/draw и non-HP update ими не являются. Существующие HP reactions не отключать ссылкой на будущий C050.
- **Snapshots отдельно от однократных реакций.** Для значимых body/form/idle transitions определить occurrence-aware SV-local interface: generation/sequence, ownership и явное consume/ack или bounded retention с явным overflow. Latest snapshot/revision не заменяет последовательность переходов между inputs. Конкретный C API выбирается при реализации; общий event bus и полные recipients C052/C053/C055 не нужны. Отложенный recipient остаётся pending, но данные о значимых промежуточных occurrences не теряются и не воспроизводятся из renderer.
- **Bounded preparation сохраняется.** `login/preparation.c:184` заканчивает обычный ingress после accepted identity. Расширение supported packets не означает чтение всех последующих HUD updates. Не снимать эту границу ради C012; собственный app/model/native caller проверяет 006 через существующий ограниченный diagnostic entry.

## Общие границы

[План](README.md), [source contract](source-contract.md) и [точные canonical записи](coverage.json) обязательны. Реализация SV-local; legacy/common менять только при необходимости текущей задачи. Модель/decoder не читает Term; UI не читает wire packets/legacy globals. Нужны атомарные complete-packet updates, baseline defaults, generation/reset и read-only views с revisions. Framing и test-only реализация поведения не заменяют production путь.

Проверки — только необходимый минимум Linux core production-пути, совместно с 007. Не создавать broad suite, новый simulator/harness или полную visual/provider matrix; расширение согласовать с пользователем. Windows/MinGW/Wine отложены. Canonical obligations/statuses сохраняются; actual adoption/close/relog проверяет C014. Собственный результат задачи нельзя заменить обещанием будущего C014 consumer.


## Реализация — 2026-11-11

Backend реализован внутри SV: `protocol/status-update.[ch]`, typed semantic view `session/status-state.h`, session-owned HUD/occurrences и app publication. Existing HP path и его отдельная revision сохранены; новые статусы не повторяют HP effects. Complete packet публикуется атомарно, absent defaults и bounded source strings сохраняются.

Production renderer/native consumer и focused evidence выполняются связанными задачами 006/007. Этот промежуточный статус не является PASS canonical obligation или пользовательской приёмкой.


## Итоговая проверка оркестратором — 2026-11-11

Linux production build, focused production status runner и bounded native HUD review прошли на финальных sources; собственный результат задачи проверен в описанных границах. Independent reviews backend/UI закрыли найденные дефекты. [Команды, артефакты и disposition всех 81 obligations](../../sv-c012-evidence.md). Actual C014 lifecycle, C013 карта и поздние recipients не получают PASS. Это технический результат, не пользовательская приёмка и не изменение canonical registry.
