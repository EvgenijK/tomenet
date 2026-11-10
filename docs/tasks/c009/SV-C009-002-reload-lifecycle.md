# SV-C009-002 — completion reload, Guide metadata и audio warning

Статус: **accepted** — Linux core, 2026-11-10, по решению пользователя.
Labels: enhancement
Owned IDs: 0 новых IDs; интеграционная часть `capability.lua.reload` задачи 001.
Зависимости: SV-C009-001. Actual wire caller реализует C010.

## Работа

1. Дать production API C010 для accepted INIT и END completion: явные transfer
   count/result, session generation и server feature inputs. При успешном INIT
   немедленно фиксировать process-sticky признаки по `strstr(fname, "audio.lua")`
   и `strstr(fname, "guide.lua")`; basename equality не подменяет substring.
   Generation проверяется до фиксации. Rejected INIT и `-u` флаги не меняют;
   последующий failure DATA/END или disconnect их не сбрасывает. END использует
   уже сохранённые флаги; передать их только из session batch при END недостаточно.
   Названия API выбрать по реальному assembly, без общей event bus.
   Не декодировать FILE_* в C009.
2. Reload инициируется после последнего END; пока остаются transfers, Lua не
   перезапускается. Сохранить значимый baseline порядок publication → reload →
   audio warning → Guide metadata → ACK/ERR (ответ остаётся C010).
   Ошибка Guide getter не подавляет предшествующее audio warning. Отличать новое END
   событие от повторного consumption/redraw, не вводить фиктивную deduplication
   по fnum, которая подавит настоящий повторный wire END baseline.
3. При updated_guide заново получить counts и строки races/classes/skills/
   schools/spells из runtime и обновить принадлежащие SV metadata. Проверить
   размеры/типы, не копировать legacy unbounded strcpy. При failure не выдавать
   старые/пустые данные за успешно обновлённые.
4. При updated_audio вывести restart warning, не перезапускать audio backend и
   не объявлять effective mappings обновлёнными. Флаги в legacy sticky, process
   lifetime: сохранить это в application Lua owner, включая последующие batches,
   без незаметного reset на границе batch/session. Stale completion не должен
   менять флаги. Возможное исправление legacy вынесено в sv-improvements.
5. Session-specific completions проверяют generation до эффектов. Shutdown
   освобождает metadata/runtime state по ownership, не удаляет shared files.
   UI redraw/focus/resize не повторяет reload. Если caller открывает Guide,
   сохранить точный placeholder `The guide is in development` и interaction.

## Согласованное поведение при ошибке

Решение пользователя от 2026-11-10: «сохраняем поведение старого клиента».
При последнем END и нулевом числе transfers попытаться перечитать текущие файлы
даже после ошибки сохранения. Вернуть transfer failure и независимый reload
result; успешное чтение не означает успешную передачу/готовность startup.
На диске может быть старый или частичный файл: сохранность старого файла
сама по себе не обещается. Reload также может завершиться ошибкой.
C010 сохраняет ERR при failed transfer независимо от результата reload.
Подавление reload из-за failed last close не входит в выбранное поведение.
Canonical тексты не менять. Подробные источники — [source contract](source-contract.md).

## Минимальная проверка и завершение

В компактном core check 003: несколько completions → один last reload;
повторное потребление/redraw без эффекта; реальные изменившиеся Guide getters;
audio warning перед Guide metadata без live remap; sticky flags после accepted
INIT → disconnect до END → last END новой session; stale generation без эффекта; reload после failed last close с отдельными transfer/reload results.
Actual END decode/ACK bytes проверяет C010, startup readiness — C011.
Нет полного native transfer PASS от прямого API теста. Linux only.

## Результат реализации — 2026-11-10

Production core реализован агентами по поручению пользователя. Linux build,
единая focused ASan/UBSan core проверка и bounded native early-startup прошли:
[фактическое evidence](../../sv-c009-evidence.md). Runtime исполняет реальные
скрипты; transfer/session consumers C010/C011/C014 остаются в своих этапах.
Canonical statuses и planning snapshot сохранены. Задача принята и завершена
в составе C009: [решение о приёмке](../../sv-stage-c009-acceptance.md).
Поздние integrations обязательны C010/C011/C014 и не блокируют/не переоткрывают
C009 автоматически. Полная native-runtime registration остаётся отдельным workflow.
