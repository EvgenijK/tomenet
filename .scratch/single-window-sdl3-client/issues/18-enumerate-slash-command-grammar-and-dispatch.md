# Enumerate slash-command grammar and dispatch

Type: research
Status: resolved
Assignee: codex
Blocked by: 10

## Question

Какова полная грамматика slash-команд, вводимых через `cmd_message()`: какие verbs и aliases обрабатываются локально, какие изменяют client state или файлы, какие отправляются серверу через `Send_msg()`, какие аргументы, подтверждения и platform/build/version gates у них есть, и какие стабильные actions и input bindings должны представить этот текстовый командный язык?

## Answer

Полный source-backed инвентарь: [Slash-command grammar and dispatch](../research/slash-command-grammar-and-dispatch.md). Прочитан весь `cmd_message()` и вызываемые owners: все локальные verbs/aliases, точные recognizers и arguments, prompts/cancel, state/file effects, platform/build/version gates и downstream packets перечислены с evidence. Исследование привязано к текущему dirty working tree через HEAD и 13 SHA-256; общий checkout не переключался. Ссылки и source fingerprints проверены; runtime parity не заявлена.

Единого slash tokenizer нет: screenshot pre-pass предшествует colour/item substitutions, затем self-chat и смешанная exact/prefix case-семантика локальных branches. Непоглощённый текст отправляется через `Send_msg()` как `PKT_MESSAGE`, server grammar остаётся opaque/server-owned. Локальные options, audio reload и autoinscriptions имеют отдельные packet effects; Lua допускает произвольную программу, а не конечный перечень verbs. Proposed semantic action families в отчёте — вход для последующего решения, не утверждённые bindings.

Исследование уточняет прежнюю input inventory: accepted empty message не отсеивается клиентом. Source-visible `/wager` flag leak, parser spellings, transformations внутри Lua/options, expansion bounds, screenshot inversion и direct-action/history contracts требуют явного решения в [Preserve input and macro semantics](04-preserve-input-and-macro-semantics.md); вопрос этого тикета дополнен. Новых research tickets не требуется: полный server slash catalog не нужен для решения о generic forwarding.
