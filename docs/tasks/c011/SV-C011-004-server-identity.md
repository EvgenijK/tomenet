# SV-C011-004 — подтверждённая сервером identity

Статус: **accepted** — Linux preparation/core, 2026-11-11, по решению пользователя.
Labels: enhancement
Owned ID: `capability.session.read-identity` (3 obligations).

## Production путь и работа

1. Реализовать SV-local version-aware приём `PKT_CHAR_INFO` по
   `src/client/nclient.c:Receive_char_info`. Character list и birth draft не
   заменяют accepted identity. Хранить race/class/trait/sex/mode/lives/name,
   generation, revision и доступность полей в session presentation state;
   UI читает семантический view, не packet или legacy globals.
2. Сохранить пять baseline wire branches: >=4.9.2.1.0.1 — i32 mode;
   >=4.7.3 — lives; >4.5.2 — name; >4.4.5.10 — trait;
   самая старая ветка — race/class/sex/mode. Для отсутствующих полей задать
   baseline defaults (trait=0, lives=-1); отдельно определить имя из принятого
   login selection для веток без name, не оставляя имя предыдущей generation.
   Сохранить старую упаковку admin/privileged/restricted bits и effective mode.
3. Partial packet не публикует частичную identity. Проверять индексы перед
   доступом к setup tables; invalid packet даёт явный результат владельцу,
   а не out-of-bounds доступ или выдуманную identity.
4. Передать accepted identity в preparation payload 001/003 и в текущую
   native surface: пользователь должен прочитать серверные значения. Изменение
   revision не исполняет повторно startup preferences. Если identity приходит
   после startup END, сохранить границу unread stream; не поглощать весь live
   поток login decoder ради этой задачи. Применимый decoder и его production
   dispatch contract принадлежат C011, подключение непрерывного live ingress — C014.
   Для C011 нужен действующий ограниченный native preparation caller этого
   decoder: frame должен пройти от сокета до state/view, а не только через
   прямой unit вызов. Остальные bytes сохраняются в порядке и с bounded lifetime;
   искать PKT_CHAR_INFO сканированием произвольных bytes или пропускать packet
   неизвестной длины нельзя. Отсутствующий consumer не вызывает recovery
   unknown/redraw для известного baseline packet. Если последовательность ещё
   нельзя обработать, preparation возвращает явное pending/unsupported-consumer
   состояние с retained tail, а не complete identity-dependent readiness.
   После END identity не обязательно является следующим packet. Обязательный
   префикс до неё обрабатывается настоящими decoders в необходимом объёме C011;
   отсутствие такого пути блокирует завершение 004, а не переносит всю работу
   автоматически в C014. Полный игровой dispatch/lifecycle при этом не требуется.
5. Зафиксировать hook для identity-dependent preference/INS selection в 001;
   формы и последующие обновления обслуживаются их owners. При cancel/retry
   старые поля и views не принимаются новой generation.

## Необходимый минимум Linux проверки

В существующем core подходе вызвать production decoder и модель для пяти wire
branches, partial frame, повторного обновления и смены generation. Проверить
точные значения/defaults, read-only view и отсутствие повторных startup effects.
Связанный managed-peer проход preparation должен показать accepted identity,
отличающуюся от draft/list: early options → PLAY/END → хотя бы один допустимый
packet перед identity → identity → INS/hook. Проверить раздельную доступность
раннего и identity-dependent payload. Не создавать отдельный simulator или broad suite.

Готовность: production decode → semantic state → native readable view и payload,
с явным evidence для result/lifecycle/allocation-checks; integrated live lifecycle
не получает PASS вместо C014. Общие правила — [план](README.md) и
[source contract](source-contract.md). Windows проверки отложены.
Pending caller всей задачи нельзя заменить будущим подключением C014:
проверяемый native путь и readable accepted identity — собственный результат 004.

## Comments

2026-11-11 — реализация и проверка по запросу пользователя. Identity проходит version-aware decoder/model/native view. Добавлен необходимый `protocol/startup-prefix.[ch]`: social/skills/rendering/boni/mapRLE frames удерживаются FIFO с seq/type/exact bytes; already-applied app/FILE occurrences не replay. Native peer проходит post-END FILE, split prefix/RLE и identity до INS. Full live updates остаются C014.

[Финальное evidence, команды и ограничения](../../sv-c011-evidence.md).
Пользовательская приёмка не заявлена; Windows проверки отложены.


2026-11-11 — follow-up по запросу пользователя закрыть оставшиеся проверки самого C011. Actual local server подтвердил accepted Born после native birth и MOTD acknowledgement; финальный preparation readout просмотрен на Linux. Исправлены отсутствующий item/audio/unique prefix framing и перекрытие identity завершённым birth/stub; failure diagnostics не перекрываются identity/PAUSE. Повторный identity/revision и bounded partial frames проверены в production core.

Результаты и границы — [обновлённое evidence](../../sv-c011-evidence.md). Работы следующих этапов не выполнялись.


2026-11-11 — пользователь явно принял C011 и задачи 001–006 в проверенных границах Linux preparation/core. Задача завершена; [решение о приёмке](../../sv-stage-c011-acceptance.md). Исторические записи выше отражают состояние до приёмки; поздние obligations и canonical runtime statuses не повышаются автоматически.
