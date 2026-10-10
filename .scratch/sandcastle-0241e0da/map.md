# SV-B-024 birth completion slices

Immutable contract: `0ba2cedc9474a7f17f847c0bdb89d04023b0a9edefa8611f7528564eeb4f83ef`.
Ticket files remain authoritative for scope, status and acceptance ownership.

## Tickets

- [01: Complete handshake](issues/01-complete-handshake.md)
- [02: Versioned handshake](issues/02-versioned-handshake.md)
- [03: Dedicated completion](issues/03-dedicated-completion.md)
- [04: Single submission](issues/04-single-submission.md)
- [05: Authoritative reply](issues/05-authoritative-reply.md)
- [06: Guide/MOTD handoff](issues/06-guide-motd-handoff.md)
- [07: Runtime audit](issues/07-runtime-audit.md)

## Decisions so far

- [03](issues/03-dedicated-completion.md#answer): existing production foundation
  plus 01 implements dedicated completion; exact-byte native endpoint checks
  verify IDDC variants, skipped trait and PvP Maia return in both compile orders.
  Controller gates and downstream evidence retain their original owners.

- [05](issues/05-authoritative-reply.md#answer): final-play outcomes return to
  startup/failure/disconnect owners; stale native input cannot command retries;
  incremental replies and ordered trailing packets verified through endpoint.
  Full gates and MOTD/handoff keep their originating owners.
- [06](issues/06-guide-motd-handoff.md#answer): authoritative creation/play ack
  now transfers to peer MOTD and live-session handoff; actual native Guide
  callers finish birth in both choice orders, with renderer frames and peer
  transcripts available for controller review. Controller/final audit pending.

- [07](issues/07-runtime-audit.md#recovery-result): 108 controlled-peer sessions,
  actual build/core results and independent controller review recorded; fresh
  final-HEAD receipts are located as described in the recovery packet. Parent
  and deferred SV-B-025/SV-B-023 acceptance ownership remain unchanged.
