# 02: Handshake на границах baseline protocol versions

**What to build:** Игрок завершает ordinary creation с теми же значениями, а controlled peer каждой применимой baseline версии получает ровно тот формат полного handshake, который эта версия поддерживает, и подтверждает play.

**Blocked by:** 01: Полный creation/play handshake и ожидание acknowledgement.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [ ] **P1-ac2, P1-ac3, P2-3:** составить и проверить version-gate matrix из canonical baseline session-play/session-setup: strict newer-than и inclusive at-least границы; identity/trait/audio/graphics/font fields, option counts, geometry, character widths и размеры visual redefinition tables. Для каждой различимой границы проверить соседние версии и саму границу.
- [ ] **P1-ac1, P1-ac3:** для каждого формата controlled peer получает полный exact-byte handshake после legal birth, возвращает authoritative acknowledgement; production defaults и identity bytes сохраняются при различной длине payload.
- [ ] **P1-ac3, P2-4:** byte oracle независим от serializer; фактические bytes получены через production endpoint, без отдельной test-only сериализации поведения клиента. Неприменимые версии явно обосновать исходным baseline, не считать проверенными.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.
