# 19: Obtain human native review and close the Stage A checkpoint

**What to build:** Зафиксированная человеческая оценка окончательной native-сборки и обновлённая приёмка 15, основанная на свежих автоматических и ручных результатах.

**Blocked by:** [16](16-restore-mingw-and-run-wine-acceptance.md), [17](17-instrument-native-fallback-runtime-checks.md), [18](18-complete-scoped-stage-a-evidence.md). Текущий оставшийся blocker вынесен в [20](20-fix-wine-direct3d-first-submission-latency.md). Предварительную UX-проверку можно выполнить раньше; финальное подтверждение привязать к окончательной сборке.

**Status:** completed; scoped Stage A accepted, 62/62 automated checks and current human review approved (2026-09-23)

**Closes blocker:** Нет human review читаемости, keyboard/cancel/focus и воспринимаемого отклика. Финализирует [15](15-run-and-report-stage-a-acceptance.md).

## Agent preparation

- [x] Подготовить запуск текущего synthetic SV в isolated profile без аккаунта/сервера; предъявить SHA-256 executable, build ID, OS/Wine/backend identity, actual logical/output dimensions и display scale. Дать короткий сценарий проверки и понятный способ перезапуска.
- [x] Обеспечить проверку читаемости HP, двух одинаковых сообщений и prompt; обычного ответа и Escape cancellation; возврата focus, resize и сохранения pending interaction; восстановления поверхности и представительного macro/lifecycle flow.
- [x] Автоматические production tests подтверждают exact reply/event semantics. Человек оценивает фактическое поведение/восприятие, а не вывод теста или screenshot. Не выдавать injected focus events за ручную проверку desktop focus.

## Human review and closure

- [x] Получить явный отзыв человека по читаемости, keyboard/cancel/focus и ощущаемому отклику в заявленных проверенных условиях. Сохранить reviewer/date, build hash, environment, выполненные действия, результаты и замечания. Молчание или истечение времени не являются approval.
- [x] Visible-response targets: urgent 50 ms, interactive 100 ms, background 250 ms только для реализованных flows. Не превращать субъективный отзыв в измеренный hard gate; actual submission timing остаётся отдельной автоматической проверкой.
- [x] Выполнить required virtual geometry matrix и честно указать физически проверенные display conditions. Не требовать дополнительный physical 4K монитор: virtual geometry разрешена контрактом. Недоступные условия не объявлять проверенными.
- [x] Если замечания выявляют дефект, исправить его в SV и повторить затронутые automated/native/human проверки на новой сборке; старый отзыв не переносить без проверки соответствия.
- [x] Привязать review record к evidence/gate из 18. При отсутствующем, отрицательном или устаревшем отзыве runner/report не сообщает accepted Stage A.
- [x] Повторно выполнить полный gate 15 и опубликовать согласованный итог. Закрыть 15 и Stage A только при прохождении всех обязательных проверок 16–18 и текущем положительном human review; иначе перечислить конкретные остающиеся blockers.

## Boundaries

Реальный login/gameplay, полнота HTML, новые HTML UX approvals, actual Windows 10/11 checkpoints B/E/F, Fedora41 shipping ABI и релизные архивы не добавляются в критерии A. B–F остаются pending. Никаких stress/soak, recording, global memory ceiling или pixel-perfect сравнений.

## Sources

`docs/sv-stage-a-acceptance.md` (manual launch и текущие ограничения), `docs/sv-lifecycle.md`, `docs/sv-geometry-timing.md`, Stage A parent spec и общий acceptance contract.

## Implementation preparation — 2026-09-23

See [manual review workflow](../../../docs/sv-human-review.md): isolated launcher,
F5/F6 fixture controls, representative m→Y macro, native geometry/backend/build
identity and explicit feedback record. Runner validates fresh binary/resource/host
identity before any scoped acceptance. Negative/absent/stale reviews never accept.
No human approval has yet been received. Ticket 15 and Stage A remain open.

Fresh full gate completed: 61/62 commands pass, all eight scoped A outcomes
covered by current evidence. Wine Direct3D first urgent submission fails at
37.719 ms / 20 ms. Human review remains pending. No Stage A closure is claimed.

## Explicit human confirmation — 2026-09-23

User: «выглядит нормально», followed by «Да, все перечисленные действия работают
без замечаний» for the manual action checklist. Review approved for the exact
Linux software build and observed conditions; see the tracked manual review
record and supplemental assessment. The sole remaining closure blocker is Wine
Direct3D timing (37.719 ms / 20 ms); Stage A is not accepted.

## Final acceptance — ticket 20, 2026-09-23

Production startup fix and final evidence: [report](../../../docs/sv-direct3d-startup.md).
Full gate passed 62/62, all eight scoped A outcomes covered, current explicit human
review approved. Wine Direct3D maximum urgent 1.057 ms / 20 ms; all delayed controls
still reject intentional violations. Stage A is accepted; B–F and full capabilities
remain pending. Earlier results above are historical and are not erased.
