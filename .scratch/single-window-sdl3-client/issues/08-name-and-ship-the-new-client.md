# Name and ship the new client across platforms

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 15

## Question

Использовать [Platform deltas and packaging: статический аудит](../research/platform-deltas-and-packaging.md): выбрать minimum Linux system ABI/Windows OS, binary/archive/config identities и coexistence paths, общий SDL3 storage root с разделением common и SV-specific configuration. Определить mandatory/optional dependencies штатного пакета, допустимость manual degradation/external helpers, package contents и launch layout; импорт старого профиля не должен незаметно менять network fingerprint, зависящий в SDL3 от user path. Текущие архивы и одноимённые legacy binaries не являются уже выбранными именами нового target.

Как называются frontend, compile-time boundary, Linux/Windows binaries, packages и user-data namespace, и какой порядок Linux-first разработки приводит к обязательной parity в текущей Linux amd64 и Windows MinGW32 release matrix без перезаписи legacy build artifacts?

## Historical discussion

Ответы уточнялись по ходу сессии; текущий подтверждённый контракт находится в Answer ниже.

### Round 1 — user answers

- Q1: рабочее имя `tomenet-sv`; может поменяться по согласованию с maintainer игры. Exact frontend flag, Windows extension, archive/profile namespace и rename compatibility ещё не утверждены.
- Q2: приняты распаковываемые Linux `.tar.bz2` и Windows `.zip`, с baseline package layout как в текущем SDL3 release recipes; пользовательские данные отдельно, общие SDL3 resource roots согласно закрытому settings/migration решению.
- Q3: штатный релиз включает PNG screenshots, audio pack extraction и automated guide download/checksum со всеми необходимыми dependencies; reduced optional-feature builds допускаются для разработки с обозначением ограничений. External 7z/wget для этих штатных функций не требуются.
- Q4: Linux-first UI implementation; MinGW32 target поддерживается с первого этапа, Windows runtime проверяется после крупных изменений, обе платформы проходят acceptance до завершения реализации.

Тикет остаётся открыт; остальные решения и итоговое общее понимание ещё обсуждаются.

### Q5 — build boundary clarification

- Пользователь уточнил: новый UI работает в новом клиенте через отдельный executable. Отдельный frontend/build target связывает собственные UI files и переиспользуемые core/SDL3 adapters; legacy executable остаётся самостоятельным.
- Предложенный глобальный `USE_SV_CLIENT` не утверждён и не является обязательным контрактом новой сборки. Необходимость узких compile-time switches в shared code решается по фактической границе модулей; отдельный executable сам по себе ни требует их, ни исключает.
- Рабочее имя `tomenet-sv` утверждено; предложенные exact archive/build/storage names и Q6–Q9 ещё требуют ответов.

### Q6 — storage root correction from user

- Пользователь требует тот же путь, что у текущего SDL3: existing SDL3 storage root (`SDL_GetPrefPath("TomenetGame", "tomenet")` либо текущий `TOMENET_SDL3_USER_PATH` override). Предложенные application name `tomenet-sv` и отдельный `TOMENET_SV_USER_PATH` не приняты.
- Пользователь уточнил ownership: одинаковые настройки используются совместно в тех же файлах, SV-only настройки отдельно. Закрытый [Define settings and migration boundary](05-define-settings-and-migration-boundary.md) пересмотрен согласно этому ответу; независимая копия common SDL3 settings отменена. Exact SV-only configuration fields/names и compatible mixed-file write contract принадлежат persistence schema ticket.

### Storage consequence — existing fingerprint path

Использование прежнего SDL3 user root с прежним SDL3 алгоритмом сохраняет path input network fingerprint; отдельный SV-only config filename и rename binary не меняют этот вход. Дополнительный алгоритм или идентификатор SV не вводится; runtime evidence остаётся в acceptance ticket. Q9 поэтому не требует отдельной смены policy. Q7 support OS, Q8 multi-install/no-install selection и exact archive names пока не отвечены.

### Q7/Q8 — confirmed by user

- Supported Linux target: amd64 на текущей Fedora41 builder baseline; без отдельного обязательства снизить system ABI. Supported Windows target: Windows10/11, i686 MinGW32 executable. Конкретные ABI/system-library/runtime closure checks задаёт acceptance ticket; успешный link не заменяет работу на выбранных OS.
- Game installation resources default: `lib/` рядом с SV executable, как текущий SDL3; стандартные явные path overrides следуют применимому baseline. Автоматический поиск других установок не вводится. При colocated installation с текущим SDL3 adjacent `lib/` общий; при раздельной распаковке у каждого архива свой bundled `lib/`.
- User resource/settings storage всегда общий current SDL3 root и override, независимо от размещения binaries. Earlier предложение выбирать найденные installations автоматически заменено уточнённой adjacent-lib схемой, принятой пользователем.

## Answer

Рабочее имя `tomenet-sv`, с возможным maintainer-agreed rename. Linux binary `tomenet-sv`, Windows binary `tomenet-sv.exe`; самостоятельная сборка нового UI с reused core/SDL3 adapters, отдельные object/output paths, без обязательного глобального `USE_SV_CLIENT`. Подтверждённые имена архивов: `tomenet-sv-<version>-linux-amd64.tar.bz2` и `tomenet-sv-<version>-win32.zip`.

Распространение и installation layout следуют текущим SDL3 recipes: binary/runtime libraries, standard shipped resources в adjacent `lib/`, documentation/license/guide и regional launch scripts; штатная поддержка PNG, archive extraction и automated guide update/checksum включена вместе с dependencies. Штатные pack/guide flows не требуют внешних 7z/wget; OS associations для обычного открытия file/URL сохраняют baseline prerequisites. Пакет формируется из intended stock assets; shared user data не становятся содержимым release archive. Exact closure/manifests и runtime evidence принадлежат acceptance, не выводятся из одного directory copy.

Linux-first UI development с MinGW32 target с первого этапа; Windows runtime checks после крупных изменений и обе платформы проходят acceptance. Linux amd64 на текущей Fedora41 builder baseline; Windows10/11 i686. Legacy/modern executables не перезаписывают друг друга; SV release scripts/object/output filenames должны соответствовать своему target. User storage identity/override текущего SDL3 остаются прежними, как и input user path исходного network fingerprint; только SV-only/UI configuration отдельна внутри общего root. Переименование binary не меняет storage identity автоматически.

Shared settings и migration contract — [Define settings and migration boundary](05-define-settings-and-migration-boundary.md); exact per-key/write/UI schema — [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md); secret storage — [Define credential storage policy](24-define-credential-storage-policy.md); actual ABI/closure/build/runtime evidence — [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md). Факты min-platform: [Факты для минимальных платформ нового клиента](../research/minimum-platform-facts.md).

Пользователь подтвердил итог вместе с именами архивов; тикет resolved. Implementation/build/release operations не выполнялись. Sharp вопросы exact persistence/credential/evidence остаются в указанных тикетах; resource-budget и terminal-fallback fog без изменений.
