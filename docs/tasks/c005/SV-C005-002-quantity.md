# SV-C005-002 — quantity core с семантикой behavior baseline

Статус: **accepted**, 2026-11-09, по решению пользователя.
Labels: enhancement
Owned IDs: `capability.input.quantity` (existing C005 allocation).
Зависимости: существующие production input router и text-field SV. Поздние
игровые callers принадлежат C017/C022/C027/C055, macro route — C053;
их отсутствие не блокирует реализацию и готовность собственного core C005.

## Проблема и источники

SV пока предоставляет `input/input.[ch]`, `input/text-field.[ch]` и native input
adapter, но не production quantity interaction. Общий text editor не реализует
caller default, ограничения, сокращения и caller-specific трактовку нуля.
Нужен неблокирующий quantity core внутри SV с явным состоянием взаимодействия;
нельзя вызывать терминальный `get_string` или переносить legacy globals в UI.

Нормативны [CONTEXT](../../../CONTEXT.md), [архитектура](../../sv-architecture.md),
[ADR-0001](../../adr/0001-separate-session-interactions-and-ui.md),
[ADR-0002](../../adr/0002-main-thread-state-ownership.md),
[ADR-0003](../../adr/0003-explicit-module-connections.md),
[ADR-0005](../../adr/0005-session-scoped-work.md) и
[ADR-0006](../../adr/0006-errors-and-overload.md).
Baseline: `src/client/c-util.c:c_get_quantity` (5058–5164), `askfor_aux` (3250,
3343), `get_string` (3464), `src/common/defines.h:PY_MAX_GOLD` (2434).
Строки — навигация текущего дерева, перед реализацией пересверить.

Canonical [native-coverage](../../capabilities/native-coverage.json) задаёт
conditions и все три `obligation.input.quantity.*`:
`result`, `lifecycle`, `allocation-checks`. Учитывать поддерживаемые версии
сервера и применимые baseline compile guards. В manifest уже указано верное
различие Escape и accepted empty. Историческое требование correction в scope
не означает необходимости повторно менять manifest; перед приёмкой сверить
его действительную формулировку и evidence, сохраняя ID и allocation.

## Реализация

1. Добавить узкий production интерфейс quantity в SV input/interactions:
   начало с `predef`, `max`, prompt и caller policy для all shortcut; поступление
   текста/edit/accept/cancel; read-only состояние и однократный результат.
   Числовые правила принадлежат quantity core, UI представляет draft и передаёт
   смысловой ввод. Использовать существующий text-field там, где его семантика
   подходит; quantity-specific адаптация остаётся внутри SV. Блокирующий цикл,
   новый общий framework и изменения legacy/common для этой задачи не нужны.
2. Сохранить `QUANTITY_WIDTH=10`, initial default и замену default первым
   обычным вводом. Подтверждение очищенной строки даёт caller default; оставленный
   initial default разбирается как текст (в частности, `-1` даёт `0`); Escape возвращает `0`, а не default.
   Для accepted empty при `predef < 0` результат — `PY_MAX_GOLD` (2000000000).
   Верхний предел применяется при `max >= 0`; `max=-1` означает отсутствие
   caller maximum; нижняя граница результата — `0`. Default и all также
   проходят соответствующие ограничения. Не вводить обязательный минимум 1.
3. Воспроизвести baseline числовой разбор: ordinary integer, регистронезависимые
   `k/M/G`, сокращённая дробная часть (`1m35` → 1350000), ведущий factor с цифрой
   (`k5` → 500), дополнение fractional tail нулями и baseline предел чтения
   fractional digits. Не подменять правила обычным decimal/float parser и не
   принимать знак минус как поддержку отрицательных quantity.
4. Сохранить all letters: `a` и прочие буквы означают `max`, либо `PY_MAX_GOLD`
   при отрицательном max; factor letter считается числовым prefix только если
   за ним следует цифра. Различать parser shortcut и мгновенное завершение
   ввода: baseline `inkey_letter_all` при пустом draft принимает `a/A/SPACE`
   немедленно как all; другие буквы требуют подтверждения. Автоматический
   prompt (`prompt=NULL`) включает этот режим сам, custom prompt получает его
   от caller. Передать это явно, без глобального флага и без включения SPACE-all
   для каждого произвольного quantity prompt. Reset/завершение очищает режим.
5. Подключить primitive к существующему production semantic input пути:
   взаимодействие сохраняется между проходами main loop, принимает edit и
   terminal input, возвращает результат один раз и восстанавливает parent
   input context. Не ограничиваться standalone parser, который production
   adapter не вызывает. Точный C API выбирается при реализации по текущему
   input seam; отдельная реализация поведения для теста недопустима.
6. Поддержать reset/cancel незавершённого ввода без повторной выдачи результата.
   Focus/resize/redraw и interleaved updates не подтверждают draft и не
   повторяют действие. Caller/session owner привязывает запрос к generation;
   очищенный/устаревший ввод не должен завершить следующее взаимодействие.
   Actual session teardown/handoff интегрируют C011/C014; C005 проверяет свой
   production lifecycle интерфейс без ожидания этих будущих routes.

Не воспроизводить неопределённое поведение C арифметики как контракт: baseline
использует signed multiplication/`atoi`, поэтому особенно внимательно сверить
ввод около s32 limits, не расширяя gameplay диапазон и не добавляя незаявленные
правила normalisation. Возможные улучшения и дефекты legacy фиксируются
отдельно в [записи об arithmetic limits](../../sv-improvements.md#define-safe-legacy-quantity-arithmetic-at-numeric-limits),
без исправления legacy в C005.

## Границы и последующие владельцы

C005 выдаёт числовой результат и сохраняет primitive semantics; `0` не означает
универсальный запрет отправки команды. Следующие owners обязаны вызывать этот
же production core и независимо проверять свои numeric/wire/lifecycle branches:

- C022: item quantities, drop/take-off и остальные фактические quantity callers.
  `cmd_drop` посылает `Send_drop` даже с quantity 0; verified stack take-off на
  server >4.4.7.0.0.0 передаёт 0 в `Send_take_off_amt`; drop-gold при 0 ничего
  не посылает. Defaults `!G`, whole-stack и item-selector cancel задаёт caller.
- C027: ordinary store buy/sell/home, доступный stock и affordability.
  Normal buy отменяется при 0, TV_GOLD branch может отправить 0; sell и
  home deposit сохраняют свои zero branches. Не объединять их в одну отмену.
- C017: `Receive_request_amt` / `Send_request_amt(id, result)`, включая 0;
  это server request caller, не автоматически покрытый ранними B receivers.
- C055: SFX/music browser index prompts и собственная интерпретация результата.
  C053: macro-derived quantity input после появления runtime.

Источники callers: `src/client/c-cmd.c:cmd_drop/cmd_drop_gold/cmd_take_off`,
`src/client/c-store.c:store_purchase/store_sell/store_process_command`,
`src/client/nclient.c:Receive_request_amt`, `src/client/snd-sdl3.c` browser index
prompts. При подключении сверять реальные функции и canonical owner конкретной
ветви: prerequisite edge к quantity сам по себе не доказывает вызов primitive.
Прочие игровые quantity callers остаются у владельцев соответствующих outcomes.

## Минимальная Linux проверка и готовность

Добавить только необходимый компактный core набор, вызывающий тот же production
quantity interaction и настоящий input/edit путь. Проверить predef/accepted
empty (включая negative predef), Escape→0 и typed zero, all letter и SPACE с
объявленной caller policy, factor prefix/suffix с fractional tail и max/min.
Выбрать значения, различающие ветви, без полной комбинаторной матрицы.
Один lifecycle case: незавершённый ввод переживает interleaved update и
focus/resize, reset очищает draft, последующий input не повторяет старый result.
Наблюдать конечный production numeric result и состояние взаимодействия,
а не только наличие сохранённой строки или callback, возвращающего fixture.

Только [Linux](../../agents/platform-validation.md); Windows/MinGW/Wine deferred.
Расширение проверок требует согласования пользователя. Здесь не добавляются
серверная/магазинная интеграция, полная UI matrix или новый harness.

Готовность: core доступен настоящему SV caller через production input path,
все перечисленные numeric branches и reset подтверждены минимальным evidence.
В отчёте дать disposition каждой из трёх canonical obligations: собственный
core результат/жизненный цикл подтверждены, точные packet bytes и actual
item/store/server/audio caller interpretation остаются явно обязательными у
названных поздних owners. Успех primitive не принимает этих callers, их будущие
integration checks не являются backward gate и не переоткрывают C005.

## Реализация и текущая проверка (2026-11-09)

Добавлен `input/quantity.[ch]`: неблокирующее production состояние с bounded
text-field, явным caller all policy, byte/text/edit input и однократным take.
`max < 0` оставляет caller maximum неопределённым. Арифметика вычисляется в
`int64_t`, затем ограничивается representable `int32_t` и caller max: undefined
signed-overflow artifacts legacy не воспроизводятся. Defined baseline inputs,
включая fractional tail до шести прочитанных цифр, сохраняются.

Важное уточнение baseline: initial `-1`, оставленный и подтверждённый Enter,
разбирается как текст и даёт `0`; только реально очищенная и подтверждённая
строка при negative predef даёт `PY_MAX_GOLD`.

Минимальный `tests/sv/quantity.c` вызывает production interaction и production
text-field. Проверены defaults/empty/zero/Escape, conditional SPACE и letters,
leading/trailing factors, fractional tail, ограничения, bounded width, reset и
single result consumption. Linux команда успешно завершилась `quantity: OK`:

```sh
cc -std=c11 -Wall -Wextra -Werror -Isrc/client/sv tests/sv/quantity.c src/client/sv/input/quantity.c src/client/sv/input/text-field.c -o /tmp/sv-c005-quantity
/tmp/sv-c005-quantity
```

Production app integration и её lifecycle evidence выполняются совместно с
SV-C005-003 в `app.[ch]`: quantity использует semantic begin/text/edit/take и
перехватывает active input до gameplay keymap. Итоговая готовность требует
сверки этой интеграции в общем отчёте C005; module-only проверка её не заменяет.
Late caller owners и canonical obligations сохраняются как описано выше.

Дополнение проверки: одиночный native text backquote и byte backquote проходят
production quantity cancel и дают однократный `0`, как `inkey` → Escape в
`c-util.c:1618`. Вставленный bulk text не проигрывается как серия клавиш:
`1` с последующим backquote внутри одной вставки остаётся текстом и разбирается
как `1`, как clipboard insertion в `askfor_aux:3171`. Macro WAIT96 потребляет
владелец C053 до передачи primitive; quantity не выполняет macro timing.
Повторён тот же focused quantity test с ASan/UBSan: результат `quantity: OK`,
лог `.scratch/sv-c005/quantity.log`.

Для sanitizer run использован `ASAN_OPTIONS=detect_leaks=0`: встроенный LSan
недоступен под текущим ptrace sandbox. ASan/UBSan активны; leak-check PASS
не заявляется.

## Итог совместной реализации C005

Production integration и минимальные Linux проверки завершены; подробные
команды, результаты и границы late evidence — в [общем отчёте](../../sv-c005-evidence.md).
Задача выполнена и принята пользователем в составе C005:
[решение](../../sv-stage-c005-acceptance.md). C004 и canonical registry statuses сохраняются.
