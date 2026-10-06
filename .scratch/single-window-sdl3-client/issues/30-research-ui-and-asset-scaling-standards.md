# Research UI and asset scaling standards

Type: research
Status: resolved
Assignee: codex-research
Blocked by:

## Question

Какая целостная, source-backed модель масштабирования подходит single-window SDL3 client на Linux amd64 и Windows MinGW32: как разделить OS/window logical units, framebuffer pixels, DPI/content scale, user UI scale, independent text/map font scales, map cells, tiles/masks/subtiles, raw pictures, cursor/effects и server-visible map viewport; какие SDL3/SDL_ttf, Windows и desktop accessibility conventions являются обязательными или рекомендованными; как должны вести себя fractional scaling, monitor changes, resize, filtering, custom fonts/tilesets и assets без подходящих density variants?

Исследование должно опираться на первичные источники и текущий source contract, сравнить разумные policies без выбора продукта за пользователя и сформулировать вопросы, достаточные для решения scaling policy в [Choose raster references and defect compatibility](23-choose-raster-references-and-defect-compatibility.md). Не устанавливать pixel-perfect acceptance, concrete performance budgets или layout ownership, принадлежащие другим тикетам.

## Answer

[UI and asset scaling standards for the single-window SDL3 client](../research/ui-and-asset-scaling-standards.md) establishes the source-backed coordinate model and the remaining policy choices without selecting them.

SDL window coordinates, render-output pixels, pixel density and display/content scale are distinct; a cross-platform project logical extent can be derived as `output pixels / SDL_GetWindowDisplayScale`, with window-event coordinates additionally accounting for pixel density. Windows Per-Monitor V2, Wayland fractional scale, SDL pixel-size/display-scale events and independent accessibility text scale all require relayout/rerasterization rather than OS/full-frame bitmap stretching.

The current client couples per-Term font metrics, DPI, tile size and server resize. The accepted single-window contract instead keeps UI/text and map/cell scales independent, fits the complete 66×22 or 66×44 grid, preserves custom PCF/TTF/tileset selection, and treats server dimensions as semantic cells rather than pixels. The research enumerates alternatives for composition targets, fractional grid rounding, PCF/tile filters, categorical masks, assets without density variants, TTF sizing, cursor/effect units and monitor-change transactions. It ends with 14 explicit questions for the raster ticket and structural acceptance invariants with no pixel-equality gate.
