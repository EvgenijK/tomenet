# SV-B-023 — Birth choices, backtracking и cancel

Статус: specified; реализация и runtime evidence pending.

## Пользовательский результат

Игрок проходит legal server-provided sex/race/trait/class/body/stats/mode choices,
возвращается назад по правильному owner и может завершить creation без отправки
частичного результата.

## Зависимости и граница

Зависит от [SV-B-022](SV-B-022-character-manage.md). DNA persistence перенесена
в C. Random/DNA choice semantics, встречающиеся в полном baseline outcome,
проверяются без принятия `birth.restore-dna` или `birth.save-dna`.

Guide capability принадлежит G. Если canonical birth obligation вызывает Guide
entry, в B показывается точная временная native placeholder и возвращается тот же
caller без изменения selection/draft; это проверка birth owner, не Guide coverage.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.birth.sex` | Complete the sex choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | `source.baseline.session-birth`, `source.policy.session` |
| `capability.birth.race` | Complete the race choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | `source.baseline.session-race`, `source.policy.session` |
| `capability.birth.trait` | Complete the trait choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | `source.baseline.session-trait`, `source.policy.session` |
| `capability.birth.class` | Complete the class choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | `source.baseline.session-class`, `source.policy.session` |
| `capability.birth.body` | Complete the body choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | `source.baseline.session-body`, `source.policy.session` |
| `capability.birth.stats` | Complete the stats choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | `source.baseline.session-stats`, `source.policy.session` |
| `capability.birth.mode` | Complete the mode choice with legal server-provided values; preserve highlighted selection, random/DNA choices and legal compatibility checks before advancing. | `source.baseline.session-mode`, `source.policy.session` |
| `capability.birth.quit` | Q or Ctrl-Q terminates creation through baseline quit/retry handling without sending a completed character. | `source.baseline.session-birth`, `source.baseline.session-dna`, `source.policy.session` |
| `capability.birth.backtrack-race` | Backspace from race returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | `source.baseline.session-race`, `source.baseline.session-dna`, `source.policy.session` |
| `capability.birth.backtrack-trait` | Backspace from trait returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | `source.baseline.session-trait`, `source.baseline.session-dna`, `source.policy.session` |
| `capability.birth.backtrack-class` | Backspace from class returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | `source.baseline.session-class`, `source.baseline.session-dna`, `source.policy.session` |
| `capability.birth.backtrack-body` | Backspace from body returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | `source.baseline.session-body`, `source.baseline.session-dna`, `source.policy.session` |
| `capability.birth.backtrack-stats` | Backspace from stats returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | `source.baseline.session-stats`, `source.baseline.session-dna`, `source.policy.session` |
| `capability.birth.backtrack-mode` | Backspace from mode returns to its preceding birth owner; preserve CLASS_BEFORE_RACE ordering, skipped unavailable trait/body steps and post-mode PvP Maia trait return. No completed play packet is sent. | `source.baseline.session-mode`, `source.baseline.session-dna`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Каждый legal/illegal/random choice и server-provided list, highlighted stable
   identity, keyboard/mouse navigation, confirm и quit.
2. Оба compile orders, skipped unavailable trait/body, dedicated IDDC/PvP и
   post-mode PvP Maia return; Backspace не отправляет completed packet.
3. Interleaved network/focus/resize не меняет owner или selection; disconnect и
   stale generation запрещают дальнейший send.
4. Каждый достижимый Guide entry показывает ровно `The guide is in development`
   и возвращает тот же birth caller без Guide state/network operation.

## Definition of Done

Все ветви идут через production interaction state и input router; fixed-response
wizard не принимается. Финальный send отдельно владеет
[SV-B-024](SV-B-024-birth-dna.md).
