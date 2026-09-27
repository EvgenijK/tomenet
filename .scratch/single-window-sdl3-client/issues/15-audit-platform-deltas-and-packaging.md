# Audit platform deltas and packaging

Type: research
Status: resolved
Assignee: codex
Blocked by: 01

## Question

Какова полная Linux/X11, SDL3 Linux и MinGW Windows delta matrix, включая compile-time receiver/feature consistency (`PKT_SANITY` registration против `SHOW_SANITY` implementation), external helpers, optional dependency degradation, filesystem/clipboard/network behavior, CI targets, runtime library closure, package contents и release artifacts, которые должен сохранить новый client target?

## Answer

Source-backed platform matrix отделяет Linux/X11, SDL3 Linux amd64 и SDL3 Windows i686 MinGW32 от legacy main-win и ARM64. Инвентаризированы обязательные SDL3/ttf/net/mixer modules, optional image/archive/curl+OpenSSL switches, GCU/tolua/Wine gates, command-key/text/clipboard и OS integration paths, filesystem roots и shared writable resources, SDL3_net pseudo-fd/timeout/handshake identity, активный fedora41 CI и Linux/Windows release recipes.

Ключевые расхождения: `PKT_SANITY` регистрируется независимо от `SHOW_SANITY`, но отключённый parser возвращает успех без потребления packet; в текущем default флаг включён. SDL3 без archive support предлагает manual unpack; без curl/OpenSSL — manual guide download/checksum, а checksum stub возвращает 0 без проверки. Наличие text-input enum и SDL clipboard API не доказывает IME/Unicode end-to-end. SDL3 fingerprint зависит от user path, поэтому новая profile identity требует осознанной политики.

CI подтверждает compile/link и наличие архивов, не runtime parity или optional-feature matrix. Linux closure исключает system ABI libraries и не выявляет все missing/runtime-loaded dependencies; Windows missing non-system DLL лишь предупреждает. Recipes копируют весь workspace `lib`, поэтому package assets и exact dependency closure нельзя считать отдельным проверенным manifest. Installer versions с `--atleast-version` и обновляемыми distro packages не являются exact dependency lock.

Отчёт: [Platform deltas and packaging: статический аудит](../research/platform-deltas-and-packaging.md). Root дополнительно проверил CI steps, dependency-copy recipes, адресный sanity guard, disabled archive/guide branches и source-link validity. Локально доступны compiler/cross-compiler tools, но default pkg-config не видит требуемый `sdl3-net` и optional `sdl3-image`; это observation среды, не результат сборки. Builds, downloads, упаковка и runtime smoke не выполнялись; exhaustive preprocessed receiver/configuration audit и конкретные shipped `.so`/DLL lists требуют дальнейшего evidence.

Уточнены [Name and ship the new client across platforms](08-name-and-ship-the-new-client.md), [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md), [Define encoding and glyph identity](22-define-encoding-and-glyph-identity.md), [Define credential storage policy](24-define-credential-storage-policy.md) и [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md). Sharp вопросы уже имеют владельцев; новых тикетов и прояснившихся resource-budget/terminal-fallback fog patches нет.
