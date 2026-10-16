# 09: Тестовые шаблоны Training Tower

Status: ready-for-human

## Требования

- Три небольших шаблона: spiders lair, bats nest, treasure room.
- Максимальная вероятность их выбора в Training Tower (данж 30).
- Собрать сервер в активном проектном worktree.

## Решение

Шаблоны имеют размер 7×5 и одинаковый вес 1000. Training Tower разрешает
только эти три кода и включает `F:ROOM_TEMPLATES_ALWAYS`: после первой
обычной комнаты каждая оставшаяся попытка выбирает подходящий шаблон,
сохраняя ограничения размеров, обязательное соединение входа и расходование
неудачной попытки. У других данжей сохраняется обычная частота выбора.
Пауки требуют глубины 2; bats nest и treasure room допустимы на глубинах 1–2.

## Проверка

- [x] Сервер загружает все три шаблона и allowlist Training Tower.
- [x] Обычная генерация Training Tower размещает шаблоны и их содержимое.
- [x] Сборка сервера и Standards/Spec review проходят.

## Comments

2026-10-13 — Standards: 0 findings; Spec: 0 findings. Обычная генерация
Training Tower с seed 1 дала по 28 размещений на этажах 1 и 2; на первом
проверены bats nest и treasure room, на втором все три шаблона. Проверены
основные сущности, паутина, соединение входов и очистка временных масок.
Оптимизированный сервер собран в активном worktree
`/home/svechnik/.codex/worktrees/room-template-integration/tomenet_server`.
Подробности воспроизведения: [training-tower-smoke](../verification/training-tower-smoke-README.md).
