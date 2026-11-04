# SV-LOGIN-ARCH-002 — Собрать связанные файлы входа рядом

**Статус:** implemented; адресные Linux checks passed, принято пользователем 2026-11-04.
**Дата:** 2026-11-04.
Labels: enhancement

**Зависимости:** [001](SV-LOGIN-ARCH-001-contract.md).

## Результат и scope

Выполнить минимальный физический перенос по таблице 001 в `src/client/sv/login/`:
выбор сервера, account/character selection, создание персонажа и MOTD presentation.
Сохранить внутреннее разделение model/interaction/native input/UI; точные подпапки
и состав устанавливает 001 по фактическим зависимостям. Рабочий вариант —
`login/{servers,account,characters,creation,motd,ui}`, без обязательства создавать
пустые/однофайловые уровни. В таблице 001 отдельно учесть feature-local
`input/pregame-events.h`, `ui/birth-descriptions.h`, `ui/motd.h` и
`ui/motd-colors.h`. `endpoint-run` может временно остаться entry-point фасадом
до extraction в 003; временный фасад не должен дублировать реализацию.
Общий protocol, text editing,
font/render utilities и vault автоматически в login не переносить.

## Рабочая карта переноса

Все пути относительно `src/client/sv/`; окончательную карту подтверждает 001.

| Текущие файлы | Предлагаемое размещение |
|---|---|
| `input/endpoint.[ch]`, `input/metaserver.[ch]` | `login/servers/` |
| `input/credentials.[ch]` | `login/account/` |
| `input/login-interaction.[ch]`, `input/native-login.[ch]`, `input/native-name.[ch]` | `login/characters/` |
| `session/birth.[ch]`, `input/birth-interaction.h`, `input/native-birth.[ch]`, `ui/birth-descriptions.h` | `login/creation/` |
| `session/character-setup.h` | Уточнить в 001: общий SV data contract либо `login/creation/`; учитывать consumers protocol/session |
| `ui/motd.h`, `ui/motd-colors.h` | `login/motd/` |
| `ui/endpoint-scene.[ch]`, `ui/login-shell.[ch]`, `ui/login-layout.h`, `input/native-endpoint.[ch]`, `input/pregame-events.h` | `login/ui/` |
| `input/chronicle.[ch]`, `session/pregame.[ch]`, `session/login-view.[ch]` | `login/` |

Оставить `protocol/*`, `credential/vault.[ch]`, общие `input/{text-field,confirm,key-encoding,input,native-input}.[ch]`,
`ui/{font,message-text,status}.[ch]`, `ui/flush-policy.h`, `session/{session,alerts}.[ch]`.
`endpoint-run.[ch]` остаётся native фасадом; его обязанности меняются в 003.
Извлечение `character-setup.h` внутрь login не должно создавать ненужную
архитектурную зависимость игровой модели от сценария входа ради раскладки.

Адаптировать явные includes, production objects/dependencies в `src/makefile.sv`,
актуальные callers и существующие runners, компилирующие перемещённые SV sources.
Поддержать один include root SV. Согласовать пересекающиеся пути с C001 без
изменения default dispatch, CLI или packaging. Обновить актуальное описание
размещения в `docs/sv-architecture.md`, не переписывать историческое evidence.

## Критерии завершения

- [x] Production использует перемещённые реализации; старых дублей и forwarding
      headers ради сохранения путей нет.
- [x] Перенос не изменяет алгоритмы, ownership и UI/protocol поведение.
- [x] Includes/build callers соответствуют новому размещению; сохраняется Linux build.
- [x] Существующие production-path проверки из минимального набора 001 собирают
      новый путь; assertions не ослаблены.
- [x] Diff не меняет архитектуру legacy/server/common и не затрагивает чужую работу.

## Минимальная проверка

Linux build `make -C src -f makefile.sv tomenet-sv` и адресный существующий
production-path smoke входа, выбранный в 001. Не добавлять тесты структуры
каталогов, полный исторический suite или Windows matrix. После изменения кода
обновить существующий graphify graph по AGENTS.md.

## Вне scope

Новые interfaces и перенос orchestration (003/004), CLI, live runtime/handoff,
перестройка дерева tests, выделение общих модулей, новый framework.

## Фактическая карта переноса — 2026-11-04

Пути слева относительны прежнему `src/client/sv/`, справа — текущему:

| Исходные файлы | Текущее размещение |
|---|---|
| `endpoint-run.[ch]` | `login/native-run.[ch]` |
| `session/{pregame,login-view}.[ch]` | `login/{pregame,login-view}.[ch]` |
| `input/chronicle.[ch]` | `login/chronicle.[ch]` |
| `input/{endpoint,metaserver}.[ch]` | `login/servers/` |
| `input/credentials.[ch]` | `login/account/` |
| `input/{login-interaction,native-login,native-name}.[ch]` | `login/characters/` |
| `session/birth.[ch]`, `input/{birth-interaction.h,native-birth.[ch]}`, `ui/birth-descriptions.h` | `login/creation/` |
| `ui/{motd,motd-colors}.h` | `login/motd/` |
| `ui/{endpoint-scene,login-shell}.[ch]`, `ui/login-layout.h`, `input/{native-endpoint.[ch],pregame-events.h}` | `login/ui/` |

Добавлены `login/flow.[ch]`, private `flow-internal.h`, `login/options.h`,
`login/ui/view.h` и `login/ui/options.h` для текущих ownership и представления.
`session/character-setup.h`, protocol, vault и generic input/UI остаются на месте.
Старый runner не оставлен фасадом: callers обновлены на новый путь.

Linux build и адресные production-path проверки прошли; scope и ограничения
проверки указаны в 005. Все исторические suites не переоткрывались.

Итоговая проверка и ограничения: [005 — интеграция](SV-LOGIN-ARCH-005-integration.md).

Приёмка: пользователь принял правки 2026-11-04 и поручил commit/push.
