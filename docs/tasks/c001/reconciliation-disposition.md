# C001 — source-backed disposition reconciliation

Дата: 2026-11-04. Статус: applicability resolved для C001; canonical metadata
не исправлялись. Это source disposition, не runtime evidence и не приёмка этапа.

## Граница

Проверены все девять rows текущего `docs/capabilities/reconciliation.json`,
содержащих `capability.platform.package-launch`; ниже номера — **0-based JSON
row index**, а `inventory:line` — физическая строка сохранённого inventory.
`manifest.json`, `native-coverage.json` и `stages.json` задают C001 как installed
launch одного outcome. Устаревший blanket rationale «Retain exact inventory
contract» не переносит чужие semantic consumers в package launch.

Применяется decision 38: чужие outcomes принимаются их этапами, принятие
package-launch не доказывает полноту packaged gameplay/audio/Guide. Политика
Linux от 2026-10-28 откладывает Windows, сохраняя исторические source facts.

## Пять конфликтов

| JSON row / inventory:line | Реальный behavior и production source | Владелец | Disposition для C001 |
| --- | --- | --- | --- |
| 281 / `single-window-input-loops:322` | Admin **build terrain**, не build executable. `src/client/c-cmd.c:9644` `cmd_master_aux_build`: wall/tree/floor/sign, feature/info modes, Esc; `:9692` input loop, `:9710` onward feature selection | `capability.admin.build-*`, C041. Frozen platform Linux/Windows build не владеет этим admin behavior | Неприменимо. Строка не описывает startup, dependency или package operation. Связь platform.*, включая package-launch, семантически ошибочна из-за двусмысленного слова build. Admin controls не включать в C001 и не считать принятыми по запуску архива. |
| 681 / `single-window-packet-state:466` | Conditional `USE_SOUND_2010` / `WINDOWS` playback/timing. `src/client/nclient.c:5078` casino animation: sound + Sleep/usleep; `:5583` sound receiver: thunder/weather lightning side effects после decode | Audio playback/weather C055, casino presentation/effects — свои consumers; wire decode остаётся у protocol owner | Неприменимо к package-launch result. Это эффекты полученного игрового события, а не загрузка runtime dependencies. C001 проверяет closure реально включённого startup; наличие/отсутствие audio в этапе явно указывается, audio behavior этим не принимается. |
| 976 / `persisted-settings-and-files:41` | Windows `TomeNET-<pack>{,-disabled,-volume}.cfg` overrides. `src/client/snd-sdl.c:1078-1105`: WINDOWS chooses user disabled/volume overrides; аналогично music `:1647-1665` | Pack configuration/event volume C055 и explicit compatibility import C051; Windows source behavior временно deferred | Неприменимо: эти файлы не являются автоматическим settings слоем SV startup, в stock archive не копируются личные overrides. Windows deferred не удаляет будущую import/audio семантику и не даёт ей PASS. |
| 1252 / `source-text-and-server-field-byte-contracts:88` | Name lookup returns numeric identity: `src/client/c-inven.c:523` item name editor79; `src/client/c-spell.c:523` power editor79/correspondence; `src/client/skills.c:624` action name/number lookup. E49 immunity/element и E40 polymorph принадлежат этим gameplay consumers | `items.select-name` C021; `skills.select-ability` C023; spell school/mimic/immunity/breath C024 | Неприменимо. Neither executable identity nor resource path lookup; exact input bytes/matching/IDs остаются в соответствующих игровых этапах. Launch evidence не доказывает их поведение. |
| 1805 / `sv-file-ownership-draft:39` | Old SDL/Windows audio overrides — explicit compatibility-import sources, **не** дополнительные automatic SV settings layers. Production evidence: те же `snd-sdl.c:759,1084,1103,1149,1647,1665`; актуальная ownership policy `docs/capabilities/settings-policy.md:17-41,155-159` | Audio C055, import C051; shared resource ownership не создаёт отдельного package consumer | Неприменимо. Не читать/переносить эти пользовательские файлы при обычном package startup. C001 проверяет отсутствие unintended personal profile consumption и stock inventory, а не реализует import. |

**Metadata conclusion:** все пять platform.package-launch связей являются
неудачной blanket allocation, а не недостающими C001 prerequisites. Для текущей
acceptance достаточно этого явного applicability disposition, разрешённого
SV-C001-001. Canonical JSON не менять точечно только ради получения зелёного
статуса: будущий owner audit должен удалить неподходящие platform links и
сохранить/уточнить реальные caller IDs с их obligations. Для row281 недостаточно
лишь удалить package-launch: сейчас реальные admin build owners вообще не
перечислены в этой row; необходим отдельный review её полного mapping.
Это не основание расширить C001 на C021/C023/C024/C041/C055/C051.

## Остальные четыре связи

| JSON row / inventory:line | Применимость |
| --- | --- |
| 35 / `baseline:101` | Production/test Linux/MinGW builds из `src/makefile.sdl3:321-346` — историческая build foundation. C001 пользуется собственным `src/makefile.sv`; Linux build outcome frozen A не переоткрывается. Windows deferred. Native launch обязателен отдельно от успешного compile/link. |
| 959 / `persisted-settings-and-files:24` | CFG смешивает connection/audio/graphics/Term layouts/credentials; строка предлагает split by key. В C001 релевантны только startup paths и отсутствие чтения/перезаписи unrelated legacy owners. Полный defaults/parse/split/save/import — C002 и последующие settings/import этапы. Legacy Term layouts не становятся SV contract. |
| 1204 / `platform-deltas-and-packaging:18` | Прямая package requirement: извлечённый Linux `.tar.bz2`, binary и `.so`, ресурсы, actual runtime closure. Применять утверждённые SV binary/archive identities вместо исторических `tomenet`/legacy имен. Linux amd64 Fedora41 baseline остаётся обязательной; Windows `.zip`/DLL deferred. |
| 2074 / `bundled-ttf-candidates:44` | CascadiaMono-Regular.ttf v2407.24: 575912 bytes, SHA256 `06520d032ec274fa5040b22c6f4a1d829081b24ba40b2da56dae89bf10c7b481`, OFL/license. Approved settings policy выбирает default; research «not selected here» не отменяет принятое решение. C001 подтверждает реальные bytes/license и работающий startup consumer, не принимает весь fonts C006 или settings outcome. |

Проверка `reconciliation.scenarios`: отдельного package-launch scenario нет;
existing broad/cross-block scenarios не становятся дополнительной C001 matrix.
Текущий ledger содержит две обязательные package obligations, обе должны
получить результаты либо source-backed applicability reason в runtime отчёте.
Этот disposition не заменяет `.result`/`.failure-lifecycle` observations.

## Источники и проверка

- [C001 spec](../../sv-stage-c001-spec.md) и [acceptance ticket](SV-C001-002-acceptance.md).
- [Planning sweep](../../../.scratch/sv-c001-planning/issues/01-scope-and-ownership.md).
- [Canonical reconciliation](../../capabilities/reconciliation.json), rows 35,
  281, 681, 959, 976, 1204, 1252, 1805, 2074; сохранённые inventories в
  `docs/capabilities/inventories/`, не внешние research copies.
- [Decision38](../../../.scratch/single-window-sdl3-client/issues/38-reallocate-post-b-by-functional-block.md),
  [shipping08](../../../.scratch/single-window-sdl3-client/issues/08-name-and-ship-the-new-client.md),
  [platform rule](../../agents/platform-validation.md),
  [settings policy](../../capabilities/settings-policy.md).

Graph query `C001 reconciliation package-launch` выполнен перед чтением
исходников; ответ truncated, поэтому конкретные строки и owners сверены чтением
JSON/inventories/production sources. Canonical JSON и статусы не изменены.
