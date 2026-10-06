# Enumerate every input loop

Type: research
Status: resolved
Assignee: codex
Blocked by: 01

## Question

Какова полная атомарная таблица normal/roguelike keys, shipped и пользовательских macros, raw commands, prompts и локальных state transitions во всех input loops `c-birth.c`, `c-cmd.c`, `c-inven.c`, `c-store.c`, `c-util.c`, `c-xtra2.c`, `skills.c` и SDL3 audio menus?

## Answer

Исследованные источники образуют не один input loop, а четырёхступенчатый pipeline и 52 явно различимых input contexts: SDL3 переводит physical gestures в bytes/navigation sequences; `inkey_aux()` и `inkey()` применяют macro policy и queue protocol; только `request_command()` применяет normal/roguelike keymap; `process_command()` выполняет local transition, открывает дочерний context, отправляет typed intent либо использует `Send_raw_key()` fallback.

Для нового клиента binding поэтому определяется не парой `key -> action`, а как минимум кортежем `(physicalGesture, logicalKey, keyset, macroPolicy, ownerContext) -> (transition или intent, returnContext)`. Каждый context также обязан фиксировать cancel/confirm/retry, compile/platform gates и restoration policy. Полный normal/roguelike dispatch и все десять raw-key fallthrough sites перечислены отдельно; slash-команды отделены от raw-key protocol.

Macro semantics состоит из normal `P:`, command `C:` и hybrid `H:` слоёв с разной доступностью в gameplay, modal/text и store contexts. Произвольные пользовательские macro definitions накладываются на shipped defaults в определённом порядке и должны разрешаться runtime, а не снимком defaults. В контракт входят queue delimiters 28/29/30/31, различающиеся `\wDD` и `\WDDDD`, navigation-key macro bypass, `safe_macros`, `abort_prompt` и missing-item state machine.

Legacy restoration разделено на OS focus, временную активацию `Term` и visual/modal restoration. Single-window client сохраняет только семантически нужное: вложенный prompt возвращает focus, selection/caret и presentation state owning context; внешний context возвращает их gameplay surface. Эмуляция OS window raising и legacy `Term` switching не требуется.

Исчерпывающая таблица, function-level ownership ledger, точные nested-loop key maps, shipped/user macro rules и 249 проверенных source citations находятся в [`docs/research/single-window-input-loops.md`](../../../docs/research/single-window-input-loops.md). Исследование привязано к checkout `4211671279ff820575c32239272bca68cf8762f7` и текущим uncommitted SDL3 sources; отдельная research branch не создавалась, чтобы не менять общий dirty checkout.

Аудит сделал два дальнейших вопроса точными: [Enumerate the remaining client input loops](17-enumerate-remaining-client-input-loops.md) покрывает loops вне исходного file scope, а [Enumerate slash-command grammar and dispatch](18-enumerate-slash-command-grammar-and-dispatch.md) покрывает текстовый командный язык. Оба должны быть закрыты до решения [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md).

## Comments

2026-09-14 — Уточнение из [Slash-command grammar and dispatch](../research/slash-command-grammar-and-dispatch.md): запись `empty ignored` для message context в linked inventory некорректна на client boundary. Подтверждённая пустая строка проходит через `askfor_aux()` до `Send_msg()` (или получает channel prefix); это не утверждает видимый server outcome. Деталь и evidence принадлежат новому исследованию.
