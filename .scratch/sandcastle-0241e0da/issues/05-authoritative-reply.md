# 05: Reply failures и stale generations возвращают правильный owner

**What to build:** Во время ожидания play игрок получает authoritative результат: success, rejection или disconnect возвращают управление соответствующему startup/lifecycle owner; partial reply ждёт продолжения, malformed и устаревший reply не открывают следующий surface.

**Blocked by:** 01: Полный creation/play handshake и ожидание acknowledgement.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [ ] **P1-ac5, P2-3:** independently derived baseline reply fixtures проверяют successful play acknowledgement, rejected play result, peer quit/disconnect и wrong packet/reply type; сообщения, retry/exit policy и destination owner соответствуют baseline. Character/name acceptance не подменяет play reply.
- [ ] **P1-ac5:** split reply на каждой границе не меняет owner до получения полного пакета; malformed/truncated-at-disconnect reply обрабатывается корректным failure/disconnect owner без ложного success.
- [ ] **P1-ac5, P2-4:** reply и input старого поколения после закрытия/retry не мутируют новую сессию, не открывают MOTD/handoff и не отправляют новый final request; fresh-generation successful reply работает.
- [ ] **P1-ac5, P2-3:** interleaved разрешённые packets и trailing bytes после ack обрабатываются в baseline порядке через production decoder/lifecycle, без потери остатка и блокирующего чтения UI loop.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.
