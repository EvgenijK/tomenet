# Факты для минимальных платформ нового клиента

Дата: 2026-09-14. Вспомогательная записка для **Name and ship the new client across platforms**; продуктовая политика не выбрана, runtime не проверялся.

## Что сейчас задаёт репозиторий

- Активный builder: `registry.fedoraproject.org/fedora:41`. Fedora 41 использует glibc 2.40 — [официальная toolchain change](https://fedoraproject.org/wiki/Changes/GNUToolchainF41), [Fedora glibc sysroot 2.40](https://packages.fedoraproject.org/pkgs/glibc/sysroot-x86_64-fc41-glibc/fedora-41.html).
- Installer задаёт SDL3 3.4.10, SDL3_ttf 3.2.0, SDL3_image 3.4.4, SDL3_net 3.2.0, SDL3_mixer 3.2.0. Проверка `--atleast-version` разрешает уже установленную более новую библиотеку; это не exact lock. Native extensions собираются с невендоренными системными библиотеками. curl, OpenSSL, libarchive и большинство codec/font dependencies разрешаются dnf без фиксированных версий.
- Windows target по умолчанию `i686-w64-mingw32`; flags содержат `WINVER=0x0501`. Это формат 32-битного x86 артефакта и compile-time декларация API, а не доказательство минимальной OS всего пакета. Makefile не задаёт самостоятельный единый CPU ISA floor всему dependency closure.
- Linux release исключает loader и семейство glibc из копирования. Поэтому архив с bundled `.so` зависит от системной glibc. Builder с 2.40 **не доказывает**, что каждый binary требует именно GLIBC_2.40; нужен максимум реально импортируемых версий символов по executable и всем shipped/dynamically loaded `.so`, плюс runtime на нижней границе.

Локальные источники: [installer](../../../.github/docker/fedora41/install-build-requirements.sh), [Containerfile](../../../.github/docker/fedora41/Containerfile), [makefile](../../../src/makefile.sdl3), [release closure](../../../.github/docker/fedora41/release-common.sh), [предыдущий аудит](platform-deltas-and-packaging.md).

## Upstream: что можно и нельзя заключить

- **SDL3 3.4.10** прямо заявляет поддержку desktop Windows назад до XP; WinRT/Phone/UWP исключены. Upstream описывает и i686 MinGW builds. Это поддержка SDL3, **не** всех расширений, codec DLL, curl/OpenSSL и TomeNET package. [README именно release-3.4.10](https://raw.githubusercontent.com/libsdl-org/SDL/release-3.4.10/docs/README-windows.md).
- **SDL3_net 3.2.0** требует SDL >=3.0.0; **SDL3_mixer 3.2.0** требует SDL >=3.4.0. Это версии библиотек, не минимальные Windows releases. Mixer имеет конфигурируемые codec dependencies; их сборка влияет на реальный минимум. [net CMake](https://raw.githubusercontent.com/libsdl-org/SDL_net/release-3.2.0/CMakeLists.txt), [mixer CMake](https://raw.githubusercontent.com/libsdl-org/SDL_mixer/release-3.2.0/CMakeLists.txt).
- **curl**: актуальная официальная инструкция требует минимум Vista/Server 2008. Это нельзя автоматически приписать незафиксированному Fedora41 mingw32-curl без определения его фактической версии/build config. Но обещание «XP, потому что SDL поддерживает XP» для полного нового пакета не обосновано. [curl build instructions](https://curl.se/docs/install.html#windows).
- **OpenSSL**: upstream описывает `mingw` и `mingw64`, включая Linux cross-build. Найденная инструкция не устанавливает пригодный единый Windows/CPU minimum для фактического незафиксированного Fedora41 DLL. Не следует выбирать минимум из наличия triplet. [OpenSSL 3.2 Windows notes](https://github.com/openssl/openssl/blob/openssl-3.2.0/NOTES-WINDOWS.md).
- **libarchive**: upstream подтверждает Windows/CMake build, но найденная инструкция не задаёт достаточный minimum Windows/CPU для фактического пакета и его compression dependencies. [upstream BuildInstructions](https://github.com/libarchive/libarchive/wiki/BuildInstructions).
- Для **SDL3_ttf 3.2.0 / SDL3_image 3.4.4** отдельный доказанный Windows/CPU floor в данном коротком исследовании не установлен. Наличие MinGW devel archive и сборка CI не равны проверке старой OS; FreeType/HarfBuzz/image codecs входят в вопрос closure.

## Возможные формулировки границы (не рекомендации)

- Linux: «x86_64, Fedora41-class/glibc 2.40 baseline; точный lower ABI после symbol/runtime audit». Такой выбор сохраняет builder. Более старый заявленный ABI требует целевого older sysroot/builder либо доказательства получившегося closure, а не одной переупаковки.
- Windows: «текущий i686 артефакт на выбранной desktop OS, minimum подтверждается запуском полного пакета». XP — лишь SDL capability; Vista — текущий curl source floor, не доказанный минимум текущего полного package; Windows10/11 может быть продуктовым support floor независимо от i686. 32-bit process на 64-bit OS не требует отдельного x86_64 executable, но его OS/runtime compatibility тоже проверяется.
- До обещания точной границы нужны: фиксированный dependency manifest, DLL import/API/CRT inspection, Linux symbol-version audit, проверка runtime-selected codecs и минимальной OS. Наименование архитектуры артефакта следует отделить от списка поддерживаемых OS.
