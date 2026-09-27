# 20: Fix Wine Direct3D first urgent submission latency

**What to build:** Устранить причину превышения urgent submission budget на первом кадре key-request в SV под Wine Direct3D и подтвердить исправление свежим полным Stage A gate.

**Status:** completed; scoped Stage A accepted, 62/62 automated checks and current human review approved (2026-09-23)

**Prerequisites:** Реализация и evidence из [16](16-restore-mingw-and-run-wine-acceptance.md), [17](17-instrument-native-fallback-runtime-checks.md), [18](18-complete-scoped-stage-a-evidence.md). Начало работы не требует закрытия 19.

**Unblocks:** Финальное закрытие [19](19-review-and-close-stage-a-acceptance.md), [15](15-run-and-report-stage-a-acceptance.md) и Stage A. Положительный human review текущей Linux software-сборки уже получен; он не отменяет timing failure и не переносится автоматически на изменённую сборку.

## Problem and baseline

Полный gate от 2026-09-23: **61/62 команды прошли**. Единственная ошибка — `wine-direct3d-timing_native`: первая submission prompt имеет `origin=2`, `class=urgent`, **37.719 ms при budget_ms=20**. Остальные семь timing samples этого positive-сценария не нарушили budget. Wrapper остановился после провала positive run, поэтому delayed negative control для Direct3D в этом прогоне не выполнялся.

Это воспроизводившаяся задержка доступного backend: предыдущие прогоны также фиксировали превышения. Wine software и Linux software/OpenGL проходят; все восемь scoped A outcomes имеют валидное evidence. Причина ещё не установлена: работа с font/texture, draw/submission и Wine/SDL/driver — направления измерения, а не готовый диагноз.

- Полный отчёт: [stage-a-ticket-19-2026-09-23.json](../../../docs/acceptance/stage-a-ticket-19-2026-09-23.json).
- Лог: `wine-direct3d-timing_native.log` в [архиве evidence](../../../docs/acceptance/stage-a-ticket-19-2026-09-23.tar.gz).
- Исходный PE SHA-256: `10a395f835f9640053ec7227c25fa2682af8823ec9246e88cd037e34a6879db0`.
- [Human review и оставшийся blocker](../../../docs/sv-human-review.md#human-feedback-received--2026-09-23).

## Scope

- [x] Воспроизвести на текущем production SV, сохранив executable/configuration hashes, Wine/SDL/actual renderer identity, геометрию и исходные timings. Использовать изолированный профиль и свежий Wine prefix со staging актуального PE/DLL; не подменять Direct3D software-renderer.
- [x] Измерить составляющие задержки первого изменённого prompt-кадра: подготовку текста/ресурсов, создание или обновление textures, рисование и `SDL_RenderPresent`. Проверить, какой участок действительно объясняет превышение; учитывать влияние самого измерения.
- [x] Исправить установленную причину преимущественно внутри SV. Соблюдать изоляцию из AGENTS.md, сохранить production decode/model/input/serializer semantics, порядок сообщений, отмену запроса и отсутствие terminal fallback. Независимые legacy/common улучшения оставить отдельными записями в `docs/sv-improvements.md`.
- [x] Если требуется подготовка ресурсов при старте приложения, реализовать её в обычном production lifecycle и объяснить границу измерения. Не прогревать renderer только в тесте и не выносить обработку измеряемого изменения за начало timing-интервала.

## Acceptance and checks

- [x] Первый и последующие urgent submissions проходят исходный **≤20 ms** budget, interactive — **≤50 ms**. Сохраняется actual monotonic decode/input → successful `SDL_RenderPresent`; задержка первого кадра не исключается из выборки.
- [x] Выполнить несколько независимых коротких запусков Direct3D с новым процессом/изолированным профилем, включая свежий Wine prefix. Сохранить все результаты и описать cold/warm условия; одного удачного повторного запуска недостаточно. Stress/soak не требуется.
- [x] Не повышать budget, не снижать urgency, не усреднять скрыто неудачные samples, не добавлять retries до зелёного результата и не отключать Direct3D-команду полного gate. Delayed negative control по-прежнему должен обнаруживать намеренную задержку.
- [x] Пройти затронутые production native-сценарии и регрессии software/OpenGL/Wine software, TTF/PCF geometry, request answer/cancel, сообщения и lifecycle. Тесты используют согласованные runnable SV и production-interface seams, без отдельной тестовой реализации поведения.
- [x] На окончательных исходниках/сборках повторить полный `tools/run_stage_a.py`, получить свежие dependency/runtime evidence и passed scoped checkpoint без проваленных команд. Опубликовать логи, hashes, timing samples и итог, обновить отчёты 15/19.
- [x] Проверить актуальность human review для окончательной сборки, ресурсов и host dependencies. При изменении проверенной идентичности повторить затронутую ручную проверку и получить явный отзыв; старую запись не переписывать как новое подтверждение.
- [x] Закрывать Stage A только при passed полном automated gate и текущем положительном human review. Если blocker не устранён, сохранить фактический failed результат и конкретную причину; diagnosis-only отчёт не является завершением этого тикета.

## Reproduction

Из корня репозитория; пути вывода должны быть новыми. Pinned SDK и desktop/Wine доступ описаны в `docs/sv-mingw.md`.

```sh
make -C src -f makefile.sv tomenet-sv.exe PKG_CONFIG_MINGW=/tmp/sv16-sdk-final/pkg-config-i686
/tmp/sv15-venv/bin/python -B tools/prepare_sv_mingw.py \
  --sdk /tmp/sv16-sdk-final --stage /tmp/sv20-wine-bin --binary "$PWD/src/tomenet-sv.exe"
env WINEPREFIX=/tmp/sv20-wine-prefix WINEDEBUG=-all wineboot -u
env WINEPREFIX=/tmp/sv20-wine-prefix WINEDEBUG=-all \
  /tmp/sv15-venv/bin/python -B tests/sv_timing_native.py \
  --wine --backend direct3d --binary /tmp/sv20-wine-bin/tomenet-sv.exe

/tmp/sv15-venv/bin/python -B tools/run_stage_a.py \
  --output /tmp/sv20-final-evidence --mingw-sdk /tmp/sv16-sdk-final \
  --sdk-downloads /tmp/sv16-downloads --human-review /tmp/sv20-human/review.json
```

Последняя команда предполагает подготовленный актуальный отзыв по инструкции `docs/sv-human-review.md`. Без него automated результаты можно собрать, но Stage A не принимается.

## Boundaries and sources

Не добавлять actual Windows 10/11, Fedora41 shipping ABI, live gameplay, релизную упаковку, physical 4K monitor, pixel-perfect или stress/soak требования. Wine остаётся промежуточной платформенной проверкой; B–F pending.

Вероятные точки исследования: `src/client/sv/ui/font.c`, `src/client/sv/ui/ui.c`, `src/client/sv/diagnostics/timing.c`, `tests/sv/scenarios/timing-scenario.c`, `tests/sv_timing_native.py`. Контракт: parent `spec.md`, `docs/sv-geometry-timing.md`, `docs/sv-mingw.md`; исходное предложение — [Profile Wine Direct3D first prompt submission](../../../docs/sv-improvements.md#profile-wine-direct3d-first-prompt-submission).

## Final acceptance — ticket 20, 2026-09-23

Production startup fix and final evidence: [report](../../../docs/sv-direct3d-startup.md).
Full gate passed 62/62, all eight scoped A outcomes covered, current explicit human
review approved. Wine Direct3D maximum urgent 1.057 ms / 20 ms; all delayed controls
still reject intentional violations. Stage A is accepted; B–F and full capabilities
remain pending. Earlier results above are historical and are not erased.
