# 07: Controlled-peer evidence и полный SV-B-024 acceptance audit

**What to build:** Проверяющий может воспроизвести завершение creation в production executable, увидеть точные handshake bytes и authoritative acknowledgement, сопоставить все текущие contract criteria с evidence и проверить успешную границу startup screen flow.

**Blocked by:** 02: Handshake на границах baseline protocol versions; 03: Dedicated IDDC/PvP adjustments перед final handshake; 04: Одна final отправка при повторном вводе и interleaving; 05: Reply failures и stale generations возвращают правильный owner; 06: Actual Guide caller и peer-driven MOTD до live-session handoff.

**Status:** ready-for-agent

Labels: enhancement, ready-for-agent

**Finding IDs:** None — initial decomposition; ledger repair findings не предоставлены.

**Origin:** SV-B-024, capability.birth.complete; immutable contract `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Production evidence seam: `sv_endpoint_run(SvEndpointOptions)` в native executable с controlled peer; P1-ac3/P1-ac6/P1-ac8/P2-4 → controller-review и fresh final audit. Этот тикет — финальный verification gate, не новая implementation capability.

Существующий endpoint seam доступен; прямого публичного final-play serializer API пока нет. Реализовать требуемый production path внутри SV и вызывать его через endpoint; не придумывать test-only интерфейс. Если endpoint не позволяет проверить обязательный критерий, зафиксировать seam/evidence blocker, а не заменять тестируемое поведение.

## Acceptance criteria

- [ ] **P1-ac1–P1-ac8, P2-3, P2-4:** собрать и воспроизвести controlled-peer runtime evidence production executable для ordinary/dedicated flows, версии, повторного ввода, reply failures, generation isolation, Guide actual caller и MOTD/handoff; указать применимость и реальные результаты каждого сценария. Baseline runner success или implementation readiness не заменяют evidence.
- [ ] **P1-ac3, P1-ac6, P1-ac8, P2-3, P2-4:** предоставить проверяемую трассу native caller → birth interaction → production serializer → peer → production reply owner; controller-owned review и fresh final audit привязаны к immutable digest `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef` и exact final HEAD. Автор не подменяет controller своим self-review.
- [ ] **P2-3:** `sv-build` выполняет `make -s -C src -f makefile.sv tomenet-sv`; `sv-core` выполняется настроенным Sandcastle core runner (`bash -s -- core` с предоставленным runner script). Записать реальные результаты; отсутствие runner/evidence или controller verdict отметить как pending blocker, не как pass.
- [ ] **P1-ac9, P1-ac10, P1-ac11:** сохранить originating filename и SV-B-023 dependency; DNA load/save, profile/resource workflows, real-server integration, session.enter-game и first gameplay surface остаются C. Handshake resource fields проверены в B; startup screen flow boundary не расширена.
- [ ] **P1-ac12:** `P1-entry-evidence` явно deferred, owner SV-B-025; downstream end-to-end entry verification не запускается как новая обязанность B-024 и не объявляется passing evidence этого checkpoint.
- [ ] **P2-1, P2-2, P2-7:** review подтверждает минимальные изменения legacy/shared и SV-local реализацию; намеренное дублирование ради изоляции не превращается в задачу объединения.
- [ ] **P2-5, P2-6:** если обнаружены отдельные улучшения legacy/shared/SV organization, добавить их в установленный список улучшений с problem, affected code, proposal, status и checks; не реализовывать их в этой декомпозиции. При отсутствии находок не создавать искусственные defects.
- [ ] **P2-8, P2-9, P2-10:** planning children остаются local Markdown, отдельно от originating SV implementation ticket; Labels отделены от Status, терминология и ownership согласованы с прочитанными CONTEXT и относящимися ADR. Parent не закрывается и не модифицируется.

## Границы и проверки

Каждый implementation срез сохраняет protocol/game semantics behavior baseline и SV isolation (P2-1–P2-4, P2-7), проходит sv-build и настроенный sv-core; controller-review и fresh final audit отдельно устанавливают acceptance evidence. Начинать с failing проверки public production seam, затем реализовать production поведение.

SV-B-023 — сохранённая upstream dependency, уже присутствующий production foundation, но его full acceptance/native evidence pending; он не включён в manifest как выдуманный completed dependency. При обнаружении реального отсутствующего prerequisite остановить зависимую реализацию и записать blocker.

P1-ac9–P1-ac11: DNA persistence, profile/resource workflows, real-server integration, session.enter-game и gameplay presentation не входят в этот срез. Требуемые handshake resource bytes входят. P1-ac12 / P1-entry-evidence deferred в SV-B-025, не passed. Обнаруженные улучшения записывать отдельно по P2-5/P2-6; намеренная изоляционная duplication сама по себе не finding.

Публикация тикета не означает, что implementation, runtime checks или controller audit уже выполнены. Работа по frontier начинается с 01; blocked children доступны после завершения перечисленных prerequisites.
