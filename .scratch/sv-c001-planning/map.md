# Планирование C001 — запуск установленного SV-клиента

Дата: 2026-10-31. Запрос пользователя: «запусти агентов подготовить план работ по C001».

Статус: planning complete. Планирование не является реализацией или приёмкой C001.
Canonical ownership: один outcome `capability.platform.package-launch`.
B и B2 приняты; их приёмка не переоткрывается. Текущая платформа — Linux,
Windows/MinGW/Wine deferred. Минимальные изменения преимущественно в SV;
расширение проверок требует предварительного согласования пользователя.

## Цель

Подготовить source-backed spec, implementation tickets и минимальные критерии
приёмки запуска из распакованного Linux-дистрибутива, не включая возможности
последующих этапов и не повышая implementation/evidence status в canonical JSON.

## Исследования

| Ticket | Ответственный | Вопрос |
|---|---|---|
| [01](issues/01-scope-and-ownership.md) | c001_scope | Границы, ownership и связь с reallocation |
| [02](issues/02-production-packaging-and-launch.md) | c001_packaging | Существующие production packaging/resource/launch paths |
| [03](issues/03-linux-acceptance.md) | c001_acceptance | Минимальная Linux-приёмка и evidence |

## Decisions so far

- [01: scope](issues/01-scope-and-ownership.md): один owner package-launch;
  пять reconciliation conflicts требуют applicability disposition перед приёмкой.
- [02: production](issues/02-production-packaging-and-launch.md): default endpoint
  вместо старого bare-launch отказа; отдельный SV Linux package entrypoint.
- [03: acceptance](issues/03-linux-acceptance.md): Fedora41 ABI, extracted archive,
  изолированный профиль, реальные resources/dependencies; без новой B2 matrix.

## Итоговый план

- [C001 spec](../../docs/sv-stage-c001-spec.md).
- [Два tickets и DAG](../../docs/tasks/c001/README.md).
- [Planning coverage snapshot](../../docs/tasks/c001/coverage.json).

Итоговая декомпозиция объединяет предложения агентов в один implementation
owner с package/runtime/evidence и отдельный нулевой acceptance gate.
Предварительные трёхчастные варианты в исследованиях — варианты, не новый DAG.
Fedora41 baseline и имя архива из decision 08 сохранены. Интерпретация canonical scope и decision 38:
package-launch принимается полностью для текущего функционального содержимого;
pregame-only описывает содержимое, а не частичную acceptance outcome. Final
shipping obligations сохраняются на поздних этапах. Canonical JSON, production code и статусы приёмки не менялись.
Общий REALLOC-005/006/007 не закрывается. Сборки и runtime tests не запускались.


## Финальная сверка

Packaging agent не нашёл существенных технических противоречий; уточнено
предварительное создание explicit temporary profile root. Acceptance agent
попросил не приписывать decision 38 прямое разрешение pregame-only acceptance;
формулировка исправлена на явную интерпретацию scope с запретом частичного
принятия outcome. Проверены локальные ссылки, exact-once ownership и актуальность
canonical hashes snapshot. Production tests/build не запускались.
