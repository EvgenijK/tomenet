# C010 — решение о приёмке

Статус: **accepted**, 2026-11-11.
C010 и задачи SV-C010-001–006 завершены и приняты в проверенных границах
Linux startup/core. [План и задачи](tasks/c010/README.md),
[исходное evidence](sv-c010-evidence.md),
[дополнительное evidence 006](sv-c010-006-evidence.md).

## Основание решения

После отчёта о выполнении двух рекомендованных дополнительных проверок и
подтверждения готовности этапа пользователь явно сообщил: **«Этап принимаю»**.

Приняты пять возможностей `capability.transfer.check/init/data/end/upload`:
SV-local receiver/sender и temporary ownership, original path/checksum/version
семантика, публикация и ACK/ERR, generation/cleanup, интеграция Lua owner C009,
copied results, actual startup dispatch/tick и output drain/resume.
Сохраняется выбранный baseline: last END вызывает reload даже после failed
close; transfer ERR и результаты runtime/Guide учитываются независимо.

| Задача | Решение |
|---|---|
| SV-C010-001 | accepted — actual INIT/END → application Lua owner и completion |
| SV-C010-002 | accepted — FILE framing, CHECK и checksum/version branches |
| SV-C010-003 | accepted — incoming INIT/DATA/END, publication и lifecycle core |
| SV-C010-004 | accepted — requested upload caller и ACK-paced sender core |
| SV-C010-005 | accepted — минимальное Linux core evidence и disposition 20 obligations |
| SV-C010-006 | accepted — sender saturation и actual native socket/login/FILE/Lua проверки |

Production build и связанная ASan/UBSan core-проверка прошли. Дополнительная
проверка собственной sender output queue подтвердила repeated backpressure без
потери/повтора bytes. Native loopback scenario подтвердил exact FILE replies,
публикацию Guide, однократное исполнение marker и copied outcome в OVERVIEW.
Native использовал ordinary production objects, SDL dummy/software и isolated
library/profile; sanitizer/physical-display review для него не заявляется.

## Сохранённые границы и обязательства

Редкие I/O faults и прочие непроверенные условия из evidence сохраняются как
ограничения принятого результата: EACCES/read-prefix, temp-open ENOSPC,
mid-copy/fclose failures, concurrent edits, exhaustive splits, kernel short writes.
Приёмка не превращает их в выполненные проверки и не запускает новую suite.

C011 принимает session preparation/readiness/handoff, C014 — live adoption,
relog/teardown, C041 — admin menu. Эти consumers остаются работой своих этапов,
не блокируют принятие текущего C010 и не переоткрывают его автоматически.
Ранее принятые C003/C009 также не переоткрываются.

Canonical IDs, ownership, prerequisites, conditions и 20 obligations сохраняются.
Исторические PASS/pending observations, source hashes, sandbox failure и
промежуточные runs не переписываются. Planning snapshot остаётся историческим;
canonical implementation/evidence statuses не повышаются этой приёмкой:
native-runtime registration остаётся отдельным workflow.

Текущая обязательная платформа — Linux. Windows/MinGW/Wine отложены по
[правилу платформ](agents/platform-validation.md) и не являются текущим blocker.
