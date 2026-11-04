# SV-C001-001 — Linux дистрибутив и production запуск

Статус: accepted, 2026-11-04, в составе явно принятого пользователем C001.
[Реализация и evidence](../../sv-c001-evidence.md),
[техническая сверка 002](../../sv-stage-c001-acceptance.md).
Labels: enhancement
Зависимости: —; переиспользуются принятые B/B2 startup paths.
Owned IDs: `capability.platform.package-launch` (единственный owner).

Контракт: [C001 spec](../../sv-stage-c001-spec.md), [индекс](README.md).

## Фактическая реализация

`tools/package_sv_linux.sh`, [контракт](package-contract.md),
[disposition](reconciliation-disposition.md). Обычный production dispatch после
login architecture refactor находится в `tests/sv/diagnostics/launcher.c`,
которому делегирует `src/client/sv/main.c`; parser не дублировался.
Существующий smoke точечно актуализирован. Fedora41 archive/runtime checks
выполнены по evidence; Windows deferred, final shipping scope сохранён.

## Работы и требования

1. Зафиксировать Linux amd64 package contract: reference OS/libc/loader,
   на принятой Fedora41 builder baseline, bundled/system dependencies,
   имя `tomenet-sv-<version>-linux-amd64.tar.bz2`, startup resource inventory,
   лицензии и команду воспроизводимой сборки. Не обещать portable all-Linux.
   Разобрать пять reconciliation conflicts из spec и записать source-backed
   applicability disposition либо минимальное metadata correction; audio/admin
   и прочие чужие outcomes в C001 не включать. Явно маркировать этапный пакет
   pregame-only, сохраняя final shipping obligations.
2. Добавить минимальный SV-local packaging script/target, например
   `tools/package_sv_linux.sh`, с выводом вне source tree. Имя — предложение.
   Существующий `make -C src -f makefile.sv tomenet-sv` — build foundation;
   legacy `fedora41/release-all.sh` не является SV packaging entrypoint.
3. Сделать production endpoint обычным запуском в `src/client/sv/main.c`;
   сохранить явные endpoint/synthetic paths, validation и secret handling.
   Уточнить help и старую ошибку, точечно актуализировать существующий bare
   invocation smoke. Отдельный launcher не требуется без технической причины.
4. Включить обязательные startup fonts/B2 assets и runtime dependency closure.
   Проверить существующие `SDL_GetBasePath`, `--library`/`TOMENET_PATH`,
   `--profile-root`/`TOMENET_SDL3_USER_PATH`/default profile и user-before-bundle
   font paths. Не принимать полные C002/C003/C006 outcomes; SV-local fixes только
   по фактическому дефекту запуска архива.
5. Собрать минимальное evidence извлечённого пакета по spec, включая обычный
   запуск из другого cwd, независимую runtime-среду, ресурсы, профиль, закрытие
   и применимые resource/profile/loader failures. Записать в
   `docs/sv-c001-evidence.md`, сохранив pending для непроверенного.

## Definition of Done

Production endpoint открывается из архива без служебных аргументов, source tree
и dev-only library path. Все необходимые runtime dependencies/resources
разрешены в заявленной Linux-среде; user-profile isolation и actual resource
lookup подтверждены. Архив, inventory, hashes, инструкции запуска и applicable
failure results доступны gate 002. Evidence не состоит только из synthetic run.

Использовать существующие проверки по diff и только необходимый минимум core
через production SV. Расширение suites/harness заранее согласовать; Windows
не запускать. Сопутствующие improvements записывать отдельно в
[список](../../sv-improvements.md), не включать автоматически в реализацию.

## Исследования

[Production seams](../../../.scratch/sv-c001-planning/issues/02-production-packaging-and-launch.md),
[Linux acceptance](../../../.scratch/sv-c001-planning/issues/03-linux-acceptance.md).
