# SV-B-024 — Подтверждение создания персонажа

Статус: specified; реализация и runtime evidence pending.

## Пользовательский результат

После всех обязательных birth steps клиент ровно один раз отправляет полный
versioned creation/play handshake и ждёт authoritative acknowledgement.

## Зависимости и граница

Зависит от [SV-B-023](SV-B-023-birth-choices.md). Историческое имя файла
сохранено для ссылок; DNA load/save в этом тикете больше нет и относится к C.

Handshake использует production defaults/identity bytes, требуемые полным
baseline outcome. Это не принимает profile/resource workflows, real-server
integration, `session.enter-game` или первый gameplay surface — они остаются C.

## Единственная первичная ответственность

<!-- owned-capabilities:start -->
| ID | Полный результат baseline / policy | Canonical sources |
|---|---|---|
| `capability.birth.complete` | Only after all required creation steps and dedicated mode adjustments submit final identity, stats, options, geometry and graphics/resource handshake and wait for play acknowledgement. | `source.baseline.session-play`, `source.baseline.session-dna`, `source.baseline.session-setup`, `source.policy.session` |
<!-- owned-capabilities:end -->

## Production SV проверки

1. Exact bytes/version gates для identity, stats, options, geometry, graphics,
   font/resource fields и dedicated adjustments через production serializer.
2. До завершения всех legal steps send отсутствует; double confirm, resize,
   focus и interleaved packets не дублируют final request.
3. Acknowledgement/rejection/disconnect/partial/malformed reply возвращают
   правильный owner; stale generation никогда не открывает следующий surface.
4. Guide placeholder obligation выполняется через actual caller, не через Guide
   implementation или test-only callback; после acknowledgement peer-driven
   MOTD ведёт в live-session handoff, не в gameplay surface.

## Definition of Done

Controlled peer видит точные bytes и authoritative acknowledgement. Успех
заканчивается на границе startup screen flow; gameplay presentation не требуется.
Сквозная проверка — [SV-B-025](SV-B-025-entry-complete.md).
