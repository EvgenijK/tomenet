# SV-C002-002 — settings load, aliases и первый option packet

Статус: accepted, 2026-11-07, в составе явно принятого пользователем C002.
Labels: enhancement
Зависимости: 001; initial audio boundary реализована по source contract.
Owned IDs: `capability.settings.load`, `capability.settings.aliases`.
Контракт: [spec](../../sv-stage-c002-spec.md), [полные rows](coverage.json).

Initial audio boundary разрешена узким SV startup consumer: actual device/pack
preparation и reporting, без live audio/UI C055/C056. Все obligations сохранены.

1. Установить actual production initial audio/device/pack/Send_audio consumer.
   Разобрать обе startup-audio obligations, старые B/E references и зависимости
   font/mapping load order. Сохранить решение в `source-contract.md`; при
   конфликте порядка не скрывать prerequisites внутри settings implementation.
2. Подключить `S/tomenet.cfg`, option entrypoint и global/system/character OPT
   слои в нормативной последовательности, с resolved ownership для includes.
   Legacy U CFG/OPT не наследуются. Сохранить U/B overrides, shared resource
   ownership и fingerprint U; полный file abstraction C003 не переносить сюда.
3. Перенести точные alias branches baseline: инверсия, one-to-many, discard.
   Преобразования только в памяти, без rewrite source и guessed rename.
4. Передать текущие option values в production login serializer до первой
   отправки. Заменить stub data на этом пути, сохранив slot identities,
   версии/размеры пакетов и reset/retry lifecycle. Не менять protocol semantics.
5. Минимальные production cases: различимые SV/legacy значения и layer precedence;
   каждый тип alias branch; missing/unreadable layer; character values в первом
   packet на существующих version branches; смена персонажа/retry без stale values.
   Использовать существующий peer и core cases 001, без нового harness.
6. Для actual initial audio выполнить сохранённые applicable obligations согласно
   решению шага 1. Requested/effective failures и no-device не считать успехом
   по наличию каталога пака. Неразрешённое требование остаётся pending.

Готово: loading достигает реальных callers; exact paths/values/bytes и все
obligations имеют evidence/disposition. Parser-only или stub-only проверка
недостаточна. Полные audio/preferences/font outcomes не принимаются этим тикетом.

## Реализация 2026-11-05

Фактические файлы, команды, результаты и ограничения —
[общий evidence](../../sv-c002-evidence.md),
[техническая сверка](../../sv-stage-c002-acceptance.md).
Принят пользователем 2026-11-07 по решению в технической сверке.
Canonical rows остаются pending до отдельной native-runtime registration.
