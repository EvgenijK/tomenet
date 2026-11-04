# C001 — technical acceptance review

Статус: **accepted**, 2026-11-04. Пользователь явно принял этап C001.
Приёмка включает SV-C001-001 и SV-C001-002 в описанной Linux reference
environment. Canonical JSON этим решением не менялся.

## Решение пользователя — 2026-11-04

Пользователь: «принимаю этап».

C001 принят по представленным implementation, Linux evidence и technical review.
Принят архив с SHA256, указанным ниже. Windows/MinGW/Wine остаются deferred;
границы Xvfb/Mesa software GL и pregame-only содержимого сохраняются.
Машиночитаемый native-runtime record не создавался: пользовательская приёмка
фиксируется этим документом и не выдаётся за автоматический registry PASS.

## Результат

Распакованный `tomenet-sv-4.9.4-linux-amd64.tar.bz2` открывает production server
selection UI обычным вызовом без аргументов, находит adjacent bundled resources
после relocation и завершается через обычный X11 window-close с exit0.
Архив SHA-256:
`5c03d91101978ae0b1e3cbe3ea4bfb614c9d8a1a90971e98b9aa76ba1e730d5c`.

Проверки выполнены в отдельном Fedora41 runtime image, без compiler/SDL-devel,
source checkout, сети и личного профиля, под uid/gid1000. Архив передан
read-only; failure cases меняли только копии в container `/tmp`. Все CLI runs
использовали cwd вне package root и unset `TOMENET_PATH`/LD search overrides.
Точные команды, artifact/build identities и logs — [evidence](sv-c001-evidence.md);
одноразовая orchestration и подробные наблюдения —
[run notes](../.scratch/sv-c001-runtime/runtime-results.md).

## Обе obligations

| Obligation | Наблюдение и disposition |
| --- | --- |
| `obligation.platform.package-launch.result` | SHA256SUMS извлечённого содержимого проверен полностью, verifier exit0. ELF amd64 и ldd closure подтверждены в Fedora41 runtime; bundled SDL3/TTF/FreeType/PNG/curl/libsecret и transitive libraries разрешаются из relocated package, system glibc/loader и X11/Mesa — reference environment prerequisites. Два коротких startup runs (software/OpenGL) exit0 с реальными captures. Отдельный bare invocation + WM_DELETE_WINDOW собственного PID/window exit0. Bundled B2 assets и readable startup UI просмотрены. |
| `obligation.platform.package-launch.failure-lifecycle` | User overlay font открыт прежде bundled font, unrelated CFG sentinel неизменён; unavailable explicit root-as-file даёт диагностированный overlay failure и корректный bundled fallback, sentinel неизменён. Missing обязательные TTF/PCF в throwaway package copy: exit1 с fatal resource/startup error. Invalid renderer: exit1, unavailable явно сообщён. Удалённая bundled libSDL3.so.0: loader exit127 без скрытой system SDL подмены. Обычный startup close не инициирует contact/login. Для неисполненных write/replace/concurrency/server-cancel операций применимость разобрана ниже. |

Actual path checks по `default-software.strace`, `default-opengl.strace` и
`user-overlay.strace` сохранены в [evidence](evidence/sv-c001/observations.json):

- `libSDL3.so.0` открыт из `relocated/installed/`; bundled Cascadia TTF и 17
  bundled login asset opens подтверждены; selected user-overlay Cascadia открыт
  из временного shared root (bundled Cascadia в этом случае не открывался).
- Default root lookup использует изолированный
  `XDG_DATA_HOME/TomenetGame/tomenet/`; personal SDL3 root не подключён.
- Source checkout отсутствует среди container mounts; file-open traces не
  содержат `/home/svechnik` или `/usr/include`. Stock resources проверены
  SHA256SUMS после extraction, до fault injection.
- Mesa driver/shader caches пишутся только в временный XDG_CACHE_HOME,
  captures — в results; это ожидаемые runtime/diagnostic effects, а не сохранение
  SV/legacy CFG. Утверждение полного отсутствия любых file writes не делается.

## Applicability и ограничения

Cancel для package startup — обычное закрытие окна; server-request cancellation
не исполнялся, серверное соединение/аккаунт для installed-launch не нужны.
В `login/native-run.c` до выбора endpoint выполняются root selection,
`profile_first_run` existence check и font lookup. Private profile settings,
import, save/replace и concurrent external edit в этом пути не исполняются;
их failure injection **неприменима к проверенному startup flow**, полнота C002,
C003 и C051 этим review не принимается. Не требуется изобретать fatal при
недоступном overlay: valid bundle остаётся законным fallback. SDL default
`SDL_GetPrefPath` может создавать directory; это не запись CFG.

Network отключена: metaserver/Chronicle unavailable ожидаемы и отражены в UI/log.
Secret Service/vault save, реальные credentials, account/login, gameplay и
provider write failures не исполнялись. Это не evidence их готовности.
No-login startup не создаёт stale session command/duplicate server effect;
проверки соответствующих игровых consumers остаются у их owners.

OpenGL path проверен на **Xvfb + Mesa software GL**,
`LIBGL_ALWAYS_SOFTWARE=1`; strace показывает `libGLX_mesa.so.0`,
`libgallium-25.0.7.so`, `libLLVM.so.19.1`. Это runtime coverage OpenGL backend,
не physical GPU/display/Wayland acceptance. Software renderer также выполнен.
Evidence ограничено указанной Fedora41 amd64 runtime, не всеми Linux systems.
Windows/MinGW/Wine deferred по [правилу платформ](agents/platform-validation.md),
без PASS и без блокировки текущего Linux результата.

C001 полностью проверяет launch текущего функционального пакета;
**pregame-only** ограничивает содержимое, а не долю принимаемого launch outcome.
Final package полного клиента, audio/pack extraction/Guide/screenshots и optional
integrations остаются поздним этапам и final verification. B/B2 не переоткрыты;
новая B2 visual/input matrix, suite/framework, stress или Windows checks не
добавлялись. Единственный capability owner — SV-C001-001; review002 owns0.

Пять конфликтных inventory mappings и остальные четыре связи разобраны в
[source-backed disposition](tasks/c001/reconciliation-disposition.md).
Они не скрывают дополнительных startup prerequisites. Blanket metadata cleanup
остаётся отдельным improvement; чужие admin/audio/item/spell/import outcomes
не засчитаны package evidence.

## Неуспешная первая orchestration

Первая попытка сохраняется отдельно в `/tmp/sv-c001-results-first-attempt/`
и [durable observation JSON](evidence/sv-c001/first-attempt/observations.json):
software/OpenGL startup завершились exit0, затем PID-only X11 search выбрал
временное hidden SDL window; XSendEvent получил `BadWindow`. Это дефект
одноразового close snippet, не production failure. Выбор исправлен на свой
PID + title `TomeNET SV - Server`, с повторным lookup перед отправкой события.
Полный второй запуск завершился exit0, восемь observations дали ожидаемые
результаты. История первой попытки не переписана как успешная.

## Verdict

Существенных незакрытых технических проблем package-launch в проверенной
Linux environment не обнаружено. Пользователь принял C001 2026-11-04 по представленному результату.
Canonical implementation/evidence promotion не выводится автоматически из
build/runtime PASS или пользовательской приёмки. Исторические и платформенные
ограничения сохраняются.
