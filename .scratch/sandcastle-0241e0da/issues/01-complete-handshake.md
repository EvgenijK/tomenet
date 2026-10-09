# 01: Полный creation/play handshake и ожидание acknowledgement

**What to build:** После завершения ordinary birth на текущей baseline версии игрок отправляет полный handshake и остаётся в startup screen flow до authoritative play acknowledgement. Это первый сквозной срез: native confirm → legal committed birth draft → production SV serializer → socket → controlled peer → acknowledgement → состояние ожидания следующего startup owner.

**Blocked by:** None (can start immediately).

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [ ] **P1-ac1, P1-ac4:** native production flow, использующий SV-B-023 choices, отправляет final request только после всех legal steps; незавершённый draft не сериализуется. Successful name/login status не принимается за play acknowledgement.
- [ ] **P1-ac2, P1-ac3:** controlled peer сравнивает весь handshake текущей baseline версии с независимо выведенными baseline bytes: итоговые identity/mode/body/class/race/trait, шесть committed stats, production option defaults, geometry, audio/graphics flags, font/resource strings и все требуемые visual identity/redefinition tables. Нулевые или сокращённые fixtures не заменяют production defaults.
- [ ] **P1-ac1, P1-ac5:** задержанный ответ сохраняет ожидание и обслуживание UI/network; только полный authoritative successful play reply завершает ожидание. Gameplay surface не открывается.
- [ ] **P1-ac3, P1-ac8, P2-4:** failing controlled-peer test становится passing через тот же serializer, который вызывается native executable; сохранить захваченные bytes и ack как evidence этого среза, не объявляя full acceptance.
- [ ] **P1-ac9:** сохранить зависимость от SV-B-023 и историческое имя originating ticket; его pending native/full acceptance не объявлять завершённым.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.
