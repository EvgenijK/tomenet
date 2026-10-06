# Inventory the complete behavior baseline

Type: research
Status: resolved
Blocked by:

## Question

Какая исчерпывающая, проверяемая capability inventory задаёт 100% behavior baseline нового клиента: общие и различающиеся возможности X11/SDL3, команды и macro triggers, pre-game и server-driven экраны, все типы subwindow-информации, графика, звук, сеть, файлы, настройки, платформенные и условно-компилируемые функции, а также функции, отсутствующие в HTML prototype?

## Answer

Behavior baseline — это совокупность всего внешне наблюдаемого поведения X11- и SDL3-клиентов, а не структура из десяти legacy-терминалов. Для заявления о 100% совместимости новый клиент обязан сохранить полный язык ввода, normal/roguelike keysets и произвольные macros, pre-game/game/death/relogin flows, protocol-driven и raw state, все информационные поверхности, rendering/audio/file/config side effects, условные возможности и Linux/Windows builds.

Инвентаризация зафиксировала 36 проверяемых семейств возможностей. Возможность считается сохранённой только вместе с её input contexts, prompts, cancel/confirm behavior, исходящим намерением, входящим результатом и ошибками, focus restoration, fallback, persistence и platform/build gates. HTML prototype остаётся источником подтверждённого UX, но не доказательством native parity.

Полный отчёт: ветка `research/single-window-behavior-baseline`, коммит `87ead6ff5`, файл `docs/research/single-window-behavior-baseline.md`.

Семейный baseline разрешён. Для объективного утверждения о 100% из него выделены отдельные атомарные исследования ввода, packet state, formatted/server-driven surfaces, renderer, persistence, платформ и HTML gap ledger.
