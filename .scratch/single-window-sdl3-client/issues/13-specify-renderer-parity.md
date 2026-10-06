# Specify renderer parity

Type: research
Status: resolved
Assignee: codex
Blocked by: 01

## Question

Какова атомарная renderer parity для encoding/control codes, text/cursor/cell metrics, PCF/TTF и high DPI, palettes, tiles/subtiles/two-mask composition, raw pictures, caches, animations, lighting, weather, resize/big-map negotiation и XHTML/composed-window screenshots?

## Answer

Renderer baseline инвентаризирован как наблюдаемые операции и lifecycle с compile-time, runtime, font, server-version и clock/RNG gates: byte/cell writes и color markers, PCF/TTF metrics, solid fills и cursor, palettes/lighting/animations, single/two-mask/subtile composition и filters, multi-cell raw pictures, memory/disk cache boundaries, weather restoration, server-visible resize/big-map dimensions и два screenshot products.

X11 и SDL3 уже различаются по glyph raster, cursor и outline; blanket pixel identity не является установленной baseline. Обнаружены byte-oriented Term/X11/PCF против UTF-8 SDL3 TTF, недоказанные PCF encoding ranges, cache runtime toggle и DPI unit consistency, а также unchecked screenshot update error. Такие paths обозначены как неизвестные/defect fixtures, не как проверенно работающие. Runtime acceptance и реализация не выполнялись; исследование не назначает произвольные performance или raster tolerances.

Полный source-backed отчёт и observable acceptance matrix: [Renderer parity inventory](../research/renderer-parity.md). Снимок исследования: ветка `research/single-window-renderer-parity`, коммит `b777ee0e7`, файл `docs/research/single-window-renderer-parity.md`.

Новые точные решения вынесены в [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md) и [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md). DPI/map viewport contract уточнён в [Specify surface layouts and responsive rules](21-specify-surface-layouts-and-responsive-rules.md); transient-layer evidence и readable XHTML/final composed-window screenshot contract — в [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md). Существующий resource-budget fog остаётся до interface/harness decisions; нового fog patch это исследование не прояснило полностью.
