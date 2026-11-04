# C001 CLI checks — 2026-11-04

Фактические результаты существующих проверок. Полные stdout/stderr отдельно в файл не сохранялись; доступны в tool outputs текущего agent turn. Smoke повторно ради логов не выполнялся.

## Linux build

Команда из repository root: `make -C src -f makefile.sv tomenet-sv`.
Результат: PASS, exit 0. Инкрементально пересобран `tests/sv/diagnostics/launcher.c`, executable relinked и скопирован в `src/tomenet-sv`.
Build ID: `linux-b3593594e96dcddf960a`.

## Existing smoke

Команда из repository root: `python3 tests/sv_shell_smoke.py --backend software`.
Первый вызов в sandbox: FAIL до UI, `SV endpoint startup failed: No available video device` (exit 1).
Повтор той же команды с разрешённым доступом к локальному дисплею: PASS, exit 0.
Финальная строка: `PASS: isolated shell, window/defaults, software, resources and failures; environment=Linux`.

Фактически measured runtime из stdout:

- Native video `wayland`, renderer `software`, architecture `amd64`, pointer_bits=64.
- SDL `3004016` (3.4.16), SDL_ttf `3002002` (3.2.2), FreeType `2.14.3`.
- Fullscreen output `2560x1601`, windowed `1357x1018`, display_scale `1.325`, UI100%.
- Fresh temporary root `/tmp/tomenet-sv-smoke-c_w3n_6k`, isolated endpoint profile `/endpoint`, synthetic `/scenario`; HOME/XDG paths under same temporary root.
- Default endpoint без `--endpoint`/`--synthetic`, bounded `--frames 3 --fixture-window 1024x768`: `SV endpoint selection incomplete; no contact attempted`, root остался без файлов.
- Reject mixed `--endpoint --synthetic`; reject `--hp-check` без synthetic; reject synthetic unmarked decoy profile.
- Existing synthetic fullscreen/window/review paths, invalid renderer, damaged overlay font recovery, direct FreeType PCF fallback, missing resource failure all PASS.
- Decoy legacy `tomenet.cfg` untouched; actual user profile не использовался. Native fonts читались из repository lib для этого development smoke; это не packaged runtime evidence.

Windows не запускался. Это smoke CLI изменения, не evidence dependency closure/Fedora41 и не archive acceptance.

`git diff --check -- tests/sv/diagnostics/launcher.c tests/sv_shell_smoke.py`: PASS, exit 0.
