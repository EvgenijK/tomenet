# SV-C001-002 — Приёмка запуска Linux-дистрибутива

Статус: accepted, 2026-11-04. Пользователь явно принял этап C001 после
технической сверки; [решение и отчёт](../../sv-stage-c001-acceptance.md).
Canonical registry этим решением не изменён.
Labels: enhancement
Зависимости: SV-C001-001.
Owned IDs: — (0); capability ownership остаётся у 001.

Контракт: [C001 spec](../../sv-stage-c001-spec.md), [индекс](README.md).

## Работы и Definition of Done

- Сверить фактическое evidence 001 с одним ID snapshot и обеими obligations
  `package-launch.result` / `package-launch.failure-lifecycle`. Неприменимые
  операции должны иметь обоснование; missing/stale evidence не считать PASS.
- Проверить disposition пяти reconciliation conflicts из spec и сохранение
  Fedora41 baseline, имени архива и поздних final shipping obligations.
- Проверить, что запускался именно extracted archive в объявленной runtime
  environment без checkout, dev library paths, личных профилей или секретов;
  endpoint открыт обычным вызовом, resources и dependencies найдены фактически.
- Проверить границы: B/B2 не переоткрыты, чужие outcomes не присвоены, Linux
  результаты отделены от Windows deferred; generic obligation не стала полной
  cross-platform/input/stress matrix. Полный gameplay package review — поздний
  verification gate.
- Представить пользователю краткий результат, ссылки на архив/evidence и
  ограничения заявленной Linux среды. Зафиксировать решение о C001; planned
  не превращать в accepted автоматически из-за наличия плана или build PASS.

Новая suite, новый checker framework и дополнительные прогоны сверх необходимого
минимума не предписаны. При конкретном пробеле вернуть задачу 001 с указанием
недостающего результата; изменение registry evidence/status проводить только
по фактическим подтверждениям существующим workflow.
