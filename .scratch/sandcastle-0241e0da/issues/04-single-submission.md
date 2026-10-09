# 04: Одна final отправка при повторном вводе и interleaving

**What to build:** Игрок подтверждает завершённый birth один раз; последующие confirm, resize, focus и network events не создают второго handshake и не позволяют отправить неполный или устаревший draft.

**Blocked by:** 01: Полный creation/play handshake и ожидание acknowledgement.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [ ] **P1-ac1, P1-ac4:** controlled peer видит ровно один полный final request при double confirm, held/repeated confirm, resize, focus changes и interleaved разрешённых packets до и во время delayed acknowledgement; отличать final request от keepalive/pong traffic.
- [ ] **P1-ac4:** на каждом незавершённом legal birth owner, после backtracking и при cancel/quit controlled peer не получает final request; проверять actual native input path, а не напрямую вызывать serializer.
- [ ] **P1-ac5, P2-4:** после закрытия поколения поздний input не отправляет final bytes; новое поколение может завершить собственный birth один раз. Endpoint lifecycle и public birth interaction остаются владельцами состояния, UI rendering не инициирует повторный send.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.
