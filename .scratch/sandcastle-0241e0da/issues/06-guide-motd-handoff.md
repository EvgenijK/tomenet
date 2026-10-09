# 06: Actual Guide caller и peer-driven MOTD до live-session handoff

**What to build:** Игрок возвращается из временного Guide placeholder к тому же birth caller, завершает creation и после authoritative play acknowledgement проходит peer-driven MOTD до live-session handoff на границе startup screen flow.

**Blocked by:** 01: Полный creation/play handshake и ожидание acknowledgement.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Public SV production seam для TDD: `sv_endpoint_run(SvEndpointOptions)` с controlled peer и native SDL input. Public `sv_birth_command` через birth interaction используется только для дополнительных state checks; он не заменяет endpoint evidence final send. Связь: P1-ac1/P1-ac3/P2-4 → sv-core и controller-review.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [ ] **P1-ac6, P2-4:** actual native production caller каждого обязательного достижимого Guide entry показывает ровно `The guide is in development`, возвращает тот же owner и сохраняет selection/draft; после возврата игрок реально завершает birth. Guide implementation и test-only callback не заменяют caller.
- [ ] **P1-ac1, P1-ac7:** задержанный play ack не открывает post-birth MOTD/handoff; после authoritative ack используются peer-provided MOTD и baseline continue/skip policy, финальное состояние — live-session handoff. Existing-character startup behavior сохраняется.
- [ ] **P1-ac7, P1-ac11:** actual native surface/outcome и controlled-peer transcript подтверждают порядок ack → MOTD → handoff; gameplay surface и session.enter-game не открываются.
- [ ] **P1-ac6, P1-ac8:** предоставить воспроизводимое evidence actual caller и native MOTD/handoff для controller review; headless callback assertions не считать доказательством native caller.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.
