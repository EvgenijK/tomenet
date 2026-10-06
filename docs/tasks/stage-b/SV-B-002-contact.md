# SV-B-002 — Contact/control protocol path с управляемым peer

Статус: частично реализован; полная acceptance по суженной границе B pending.
[Существующее evidence](../../sv-b002-evidence.md) сохраняется только для
актуальных IDs и должно пройти freshness review.

## Пользовательский результат

Выбранный endpoint проходит TCP contact, negotiation, verification и setup
через production protocol path. Управляемый peer воспроизводимо подаёт все
version branches, fragmentation и отказы, пока native UI остаётся отзывчивым.

## Зависимости и граница

Зависит от [SV-B-001](SV-B-001-endpoint.md). Peer является настоящей второй
стороной TCP/protocol exchange, но не заменяет decoder, serializer, session
model или UI. Интеграция с реальным TomeNET server перенесена в C.

Unknown-packet recovery, keypress stub, pause/flush/confirm/end-marker и другие
gameplay/control outcomes также перенесены в C. Этот тикет не открывает игровой
экран и не заявляет `session.enter-game`.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.connection.contact` | Complete TCP contact, version/protocol negotiation, verification and setup before exposing character selection. | `source.baseline.session-connect`, `source.baseline.session-verify`, `source.baseline.session-setup`, `source.policy.session` |
| `capability.connection.contact-failure` | Present DNS/socket/timeout, ban, incompatible-version and verification/setup failure distinctly; release the failed connection and allow only the baseline retry/exit transitions. | `source.baseline.session-connect`, `source.baseline.session-verify`, `source.baseline.session-setup`, `source.policy.session` |
| `capability.network.keepalive` | Consume server keepalive as a payload-free no-op without an acknowledgement. The independent Net_flush/Send_keepalive timer continues during nested interactions. | `source.reconciliation.network.keepalive`, `source.reconciliation.network.keepalive-send` |
| `capability.network.ping-echo` | For pong=0 reply exactly once with pong=1 and the unchanged correlation tuple/payload. For pong!=0 update lag telemetry without an echo; preserve bounded sample indexing and platform timer behavior. | `source.reconciliation.network.ping-echo` |
| `capability.network.partial-packet` | Incomplete packets wait without publishing partial state; preserve baseline rollback and ambiguous-progress clearing. | `source.reconciliation.network.partial-packet`, `source.reconciliation.network.unknown-packet` |
| `capability.network.malformed-packet` | Decode failure follows baseline clear/disconnect and releases old session state with visible failure. | `source.reconciliation.network.malformed-packet`, `source.reconciliation.network.unknown-packet` |
| `capability.network.server-flags` | Retain negotiated server feature words and update applicable command availability. | `source.reconciliation.network.server-flags` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Exact outgoing/incoming bytes для каждой поддерживаемой version layout;
   split at every boundary, chained packets, zero-progress receive и outgoing
   queue pressure без partial send.
2. DNS/socket/timeout, ban, incompatible version, verification/setup rejection,
   malformed packet and disconnect освобождают session generation и возвращают
   только разрешённый retry/exit owner.
3. Keepalive и ping обрабатываются во время каждого B surface без блокировки UI;
   pong echo ровно один, server flags атомарно видны следующим callers.
4. Reconnect нового generation не принимает старые bytes, timers или callbacks.

## Definition of Done

Checks проходят через production transport/decoder/model/serializer; fixture
управляет только peer bytes, clock и faults. Evidence фиксирует protocol version,
build/platform и expected/actual bytes. Linux и Windows paths обязательны;
real-server smoke остаётся C и не является условием B.
