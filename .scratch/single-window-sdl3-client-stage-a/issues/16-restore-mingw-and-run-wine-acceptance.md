# 16: Restore reproducible MinGW i686 builds and Wine evidence

**What to build:** Воспроизводимое cross-окружение и свежие MinGW/Wine результаты для текущего production SV, достаточные для платформенной части Stage A.

**Blocked by:** none; финальные результаты должны соответствовать сборке после изменений 17–18.

**Status:** ready-for-agent

**Closes blocker:** MinGW build не проходит dependency check; Wine текущей сборки не проверен. Разблокирует [15](15-run-and-report-stage-a-acceptance.md).

## Scope

- [ ] Восстановить изолированный SDK с закреплёнными версиями SDL3, SDL3_ttf и FreeType для i686 MinGW. Использовать существующий рецепт `docs/sv-shell.md` как отправную точку; записать происхождение/версии зависимостей, команды и необходимые runtime DLLs. Не зависеть от исчезнувших файлов прежнего `/tmp`.
- [ ] Собрать текущий `tomenet-sv.exe`, подтвердить PE32/i386, конфигурацию и SHA-256. Проверить отдельные Linux/MinGW object/configuration outputs и отсутствие перезаписи legacy binary.
- [ ] Подготовить отдельный Wine prefix, staged binary/DLL directory и изолированные SV profiles. Staged binary должен совпадать по hash с проверяемым MinGW build.
- [ ] Выполнить shell/resource smoke и production HP/messages/key-request/lifecycle scenarios; geometry с TTF/PCF и actual submission timing. Обязателен явно выбранный software renderer; проверить доступный accelerated renderer либо явно записать его недоступность. Записать фактические Wine/SDL backend, архитектуру, build ID и версии, не только запрошенные настройки.
- [ ] Подключить воспроизводимый запуск к `tools/run_stage_a.py`: новый запуск должен подготовить/использовать явно выбранное окружение без ручной подмены свежего executable устаревшей staged-копией. Не отключать проваленные проверки.
- [ ] Сохранить текущие логи/отпечатки и обновить платформенную часть отчёта 15. После изменений native-кода из 17 повторить затронутые проверки на окончательной сборке.

## Acceptance

Linux amd64 и MinGW i686 outputs проверены отдельно; обязательные Wine software checks проходят на текущей сборке, с точными reply bytes и без потери required events/состояния. Ошибка SDK, DLL, запуска или сценария оставляет gate непрошедшим.

Wine — промежуточное evidence, не Windows acceptance. Actual Windows 10/11 checkpoints остаются B/E/F. Shipping archives, Fedora41 ABI certification и установка Windows VM вне этой задачи. Legacy/common не менять без необходимости текущего SV пути.

## Sources and checks

- `docs/sv-shell.md` — SDK recipe, resource/profile ownership.
- `docs/sv-stage-a-acceptance.md` — текущий отчёт и blockers.
- `src/makefile.sv`, `tools/run_stage_a.py`, существующие `tests/sv_*_native.py`, `tests/sv_shell_smoke.py`, `tests/sv_hp_checks.py`.
- Использовать существующий production native scenario seam; отдельную реализацию поведения для тестов не вводить.
