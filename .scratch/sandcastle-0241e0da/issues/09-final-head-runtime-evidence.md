# 09: Fresh exact-HEAD evidence for birth completion

**What to build:** Проверяющий получает свежую воспроизводимую controlled-peer трассу полного creation/play handshake, authoritative acknowledgement, actual Guide caller и peer MOTD до startup live-session handoff, а также независимо проверенный receipt для exact reviewed HEAD. Это один evidence repair, без новой реализации поведения.

**Blocked by:** 08: Verify focused production coverage (completed; можно начинать). Завершённая production реализация и runtime collector доступны через этот prerequisite; новых unresolved или deferred зависимостей нет.

Type: verification
Status: open
Assignee: unassigned
Labels: bug, ready-for-agent

**Finding IDs:** D-9496281ca15e0741

**Defect key:** final-head-runtime-evidence-missing

**Origin:** SV-B-024 / capability.birth.complete. Immutable contract digest: `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.

**Production seam:** Публичный `sv_endpoint_run(SvEndpointOptions)` с native SDL input и controlled TCP peer. Accepted trace: native birth caller → committed draft → production `sv_login_start_play` serializer → peer socket → `sv_login_receive` → production login/pregame/lifecycle owner; Guide вызывается actual production caller. P1-ac3/P1-ac6/P1-ac8/P2-4 связывают этот seam с controller-review и fresh final audit. Тикет не меняет behavior и не заявляет red/green цикл; использовать существующий production collector, не test-only интерфейс. Если seam, runner или независимый controller недоступен, записать blocker и оставить соответствующую acceptance проверку pending.

## Acceptance criteria

- [ ] **P1-ac3, P1-ac6, P1-ac8, P2-4:** собрать новый controlled-peer runtime capture на reviewed HEAD `9a98594669cba32bef9d98236cf8f0902a27eba0` и указанном immutable digest. Использовать новый пустой evidence directory и существующий collector без case filters. Retained receipt включает tested HEAD, digest, время, команды, exit codes, backend, source fingerprints и raw peer requests/replies, transcripts и Guide/MOTD frames; HEAD и inputs неизменны в течение захвата. Historical RESULTS для `23684316119dfd56deb9251f743faaa0a2b96e6f` не переименовываются в свежие.
- [ ] **P1-ac1–P1-ac8, P2-3, P2-4:** существующая полная focused matrix проходит через production seam: ordinary 3, versions 40, dedicated 20, submission 18, replies 17, Guide/MOTD 10 (108 sessions). Независимое readback проверяет каждый новый artifact size/SHA-256 и source fingerprint, exact handshake bytes/version gates, legal-step/dedicated completion, отсутствие early/duplicate send, reply ownership/generation isolation, actual Guide caller с preserved draft и ack-gated peer MOTD/handoff. Исторические 347 artifact fingerprints остаются сохранёнными; их успешная проверка не подменяет новый capture. Успешный reply проверяется по raw bytes `02 03 7a`, а не ошибочному Guide transcript prose `02 03 01`.
- [ ] **P1-ac3, P1-ac6, P1-ac8, P2-4:** независимый controller проверяет новую трассу и записывает fresh final-audit verdict/acceptance receipt с exact HEAD и digest до закрытия finding. Capture, readback и verdict составляют один coherent repair D-9496281ca15e0741; авторский self-review или baseline runner success не считается controller acceptance.
- [ ] **P1-ac8, P2-3:** после integration/resolution commit выполнить новый capture и независимый final audit на получившемся exact final HEAD, даже при неизменных source fingerprints. Финальный runtime manifest, независимый verdict и acceptance receipt хранить вне Git tree в recoverable persistent location, привязанной к этому HEAD; указатель и процедура обнаружения доступны проверяющему. Не делать последующий commit, переименовывающий старые receipts в evidence нового HEAD: любой последующий commit требует собственной свежей проверки. Review HEAD и final HEAD различаются явно, если commit их изменил.
- [ ] **P2-3, P1-ac8:** свести фактические `sv-build`, configured `sv-core` и controller-review receipts для того же final HEAD/digest. Build command: `make -s -C src -f makefile.sv tomenet-sv`; core command: `bash -s -- core` с предоставленным Sandcastle runner и настроенным environment. Текущие passing build/core receipts сохраняются как evidence их собственного HEAD, collector pending fields не превращаются в pass автоматически. Недоступные checks и неудачные попытки отражены честно; finding не закрывается без обязательного runtime/controller gate.
- [ ] **P1-ac9–P1-ac11, P2-1, P2-2, P2-7:** сохранить SV-B-023 dependency, историческое имя originating ticket, baseline defaults/identity/resource handshake fields и SV isolation. Успех заканчивается на startup screen flow boundary при inactive gameplay; DNA load/save, profile/resource workflows, real-server integration, session.enter-game и first gameplay surface остаются stage C. Production/legacy/shared implementation не меняется ради устранения evidence-only finding.
- [ ] **P1-ac12:** явно сохранить `P1-entry-evidence` как deferred, owner SV-B-025; это не passing evidence checkpoint и не blocker нового тикета. SV-B-023 full acceptance не присваивается этому repair.
- [ ] **P2-5, P2-6, P2-8–P2-10:** сохранять local ticket/ledger history и отдельно Labels/Status; originating parent не изменять и не закрывать. Новые improvement opportunities, если обнаружены, записать отдельно с problem, affected code, proposal, status и checks, без включения в repair. Не пересоздавать resolved/residual findings и не исправлять этот defect другими unrelated tickets.

## Starting evidence and closure boundary

Исторические captures и совпадающие inputs поддерживают implementation correctness, но fresh final runtime gate отсутствует. Recovery устанавливает необходимость fresh audit после каждого integration HEAD; указанный там external final-audit directory отсутствует по review finding. Этот тикет устраняет именно этот пробел и сохраняет уже verified behavior. До retained exact-final-HEAD receipt finding остаётся open. Ticket 07/08 completion не является новым controller verdict; completed implementation не переделывается.
