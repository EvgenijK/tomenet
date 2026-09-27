# Define credential storage policy

Type: grilling
Status: resolved
Assignee: codex
Blocked by: 05

## Question

Как single-window client хранит перенесённый или введённый пароль на Linux amd64 и Windows MinGW32: механизм хранения, доступность при login/autologin, поведение при недоступности хранилища, замена/удаление секрета, совместимость существующего reversible representation и ограничения диагностики/экспорта? Импорт пароля включён в согласованный состав переноса; выбор способа дальнейшего хранения пользователь явно оставил отдельным решением в [Define settings and migration boundary](05-define-settings-and-migration-boundary.md).

Использовать credential facts из [Persisted settings and files: source inventory](../research/persisted-settings-and-files.md) и platform/build facts из [Platform deltas and packaging: статический аудит](../research/platform-deltas-and-packaging.md). Целевой Windows frontend — SDL3 i686 MinGW32 на Windows10/11 согласно [Name and ship the new client across platforms](08-name-and-ship-the-new-client.md); legacy main-win, наличие Wine и успешный cross-link не доказывают доступность выбранного secret-storage mechanism. OS-specific API facts исследовать по первичным источникам.

Уточнение пользователя 2026-09-18: для SV выбран отдельный защищённый secret store. Совместимость password read/write со старым SDL3 не требуется; открытый пароль в старом конфиге может оставаться, его устранение решается вне нового клиента. Это явное исключение из общей shared-settings policy. Требуется определить migration/export semantics и исключить запись SV password через общие legacy writers. Credentials не помещаются в presentation events. Результат — planning policy, не реализация.

## Comments

### Source clarification — 2026-09-18

Статическая повторная проверка уточнила исходный вопрос: штатный SDL3 CFG хранит открытый пароль. `my_memfrob` — обратимое runtime/protocol representation, которое writer снимает перед записью. Успешный login вызывает автоматическое сохранение credentials; `fullauto` управляет пропуском login UI отдельно и в создаваемом CFG закомментирован. Evidence и исправление прежней формулировки — [Persisted settings and files: source inventory](../research/persisted-settings-and-files.md#credentials-и-server-updates). Пользовательские секреты не читались. Storage policy ещё не выбрана.

### Round 1 — confirmed by user, 2026-09-18

- Q1: для нового клиента требуется защищённое хранилище. Пользователь явно принимает, что старый клиент не сможет с ним работать; открытый пароль в старом конфиге может оставаться, его устранение решается вне нового клиента. Рекомендация сохранить общий открытый формат отклонена.
- Q2: принято автоматическое запоминание после успешного входа; автовход управляется отдельно и по умолчанию выключен. Ошибка входа возвращает ручной ввод, сохраняя прежнюю запись.
- Q3: пароль исключён из экспорта по умолчанию, включение допускается явно для переноса. В диагностике, логах и presentation events секрет и его обратимое представление исключены всегда.
- Pending: OS mechanism, unavailable/locked/failure behavior, credential identity and lookup, import precedence, replacement/deletion lifecycle, precise export representation. Исследование API не утверждает эти решения за пользователя.

### Additional source facts for the next round

- Password change не имеет отдельного machine-readable reply: [Receive_change_password](../../../src/server/nserver.c#L16047) вызывает обработчик, который сообщает успех/ошибку обычным текстом через [msg_print/msg_format](../../../src/server/party.c#L6064). Клиент меняет runtime password после отправки, без подтверждённого сохранения сервером ([c-util.c](../../../src/client/c-util.c#L13153)). Свежий успешный login подтверждает пригодность введённого пароля; уже авторизованная сессия может пропускать проверку ([nserver.c](../../../src/server/nserver.c#L5113)). Policy замены protected record пока не выбрана.
- Account lookup использует точное сравнение ([party.c](../../../src/server/party.c#L552)); клиент trim-ит ввод и капитализирует первый символ ([c-birth.c](../../../src/client/c-birth.c#L227), [c-init.c](../../../src/client/c-init.c#L3827)), сервер также применяет name validation ([nserver.c](../../../src/server/nserver.c#L990)). Blanket lowercase всего account недопустим. DNS aliases и IP нельзя автоматически считать одной credential identity по baseline.

OS API evidence: [Secure credential storage APIs](../research/secure-credential-storage.md). Linux binary libsecret/Secret Service и Windows generic Credential Manager доступны как кандидаты; metadata не secret, Windows target matching case-insensitive, Linux clear helper не доказывает удаление locked matches. Только docs/header evidence, без vault runtime checks.

### Round 2 — original proposals (answers below)

- Q4: Linux Secret Service/libsecret; Windows generic Credential Manager для текущего OS user, persistence на этой машине; отдельного master password SV нет.
- Q5: недоступность/отмена разблокировки → ручной ввод на сессию без открытого disk fallback; ошибка сохранения не прерывает успешный игровой вход, UI честно показывает несохранённость, без циклических prompts.
- Q6: отдельная запись на server address + port + account; aliases не объединяются автоматически, account identity сохраняет baseline semantics без blanket lowercase.
- Q7: legacy password читается только при явном импорте; существующий SV secret имеет приоритет, замена лишь явно, отсутствие/удаление SV secret не запускает автоматический fallback к legacy CFG.
- Q8: «Забыть пароль» удаляет выбранную SV record и выключает автоматическое сохранение для неё до явного повторного разрешения; текущий сеанс продолжается, legacy config не меняется, сбой удаления не объявляется успехом.
- Q9: явный экспорт с секретами — зашифрованный перенос с отдельной парольной фразой либо допустимый пользователем открытый файл с явным предупреждением; рекомендация — зашифрованный. Конкретный формат потребует facts после выбора.
- Следующий зависимый вопрос: точный replacement lifecycle при смене игрового пароля без machine-readable server acknowledgement; fallback/export backend детали после этого раунда.

### Round 2 — user answers and export clarification

- Q4–Q7 приняты: Secret Service/libsecret на Linux и generic Credential Manager на Windows; session-only manual fallback при недоступности без открытого disk fallback; отдельная запись server address + port + account; legacy password только через явный import, без автоматического fallback и с приоритетом существующей SV record.
- Q8 отклонён: пользователь явно указал «не добавляем опции \"забыть пароль\"». Такая команда и связанный с ней remember-disable switch не входят в UI. Согласованное автоматическое запоминание сохраняется.
- Q9 не принят: пользователь спросил, о каком экспорте идёт речь. Агент имел в виду новую функцию выгрузки настроек с паролями для переноса на другой компьютер. В карте согласован legacy import, но отдельный credential/profile exporter не установлен; предложение зашифрованного export container было преждевременным расширением scope и снимается. Q3 фиксирует ограничение попадания секретов в outputs и условие явного выбора, но сам по себе не требует создания новой функции экспорта. Существующие message/character/screenshot exports — другие capabilities.
- Pending decision: после запроса смены игрового пароля обновлять защищённую запись только после следующего успешного свежего login с новым паролем; до него сохранять предыдущую запись и держать новый runtime candidate только в памяти. Это предложение, не утверждённая policy.

### Q10 — user decision

Пользователь отклонил ожидание следующего успешного login: «10, нет, сохраняем сразу». После подтверждения нового пароля в форме и успешной постановки запроса смены на отправку обновление защищённого хранилища запускается сразу, без ожидания server acknowledgement или следующего входа. Ниже итоговая резолюция заменяет pending proposals выше.

## Answer

Решение согласовано последовательными ответами пользователя на Q1–Q10, включая отклонение «Забыть пароль» и немедленное сохранение при смене пароля. Тикет разрешён 2026-09-18. Это planning policy; код клиента, реальное чтение секретов и runtime acceptance не выполнялись.

### Storage и ownership

SV использует защищённое системное хранилище текущего OS user: Linux amd64 — Secret Service через libsecret с binary secret values; Windows10/11 i686 MinGW32 — generic Credential Manager (`CRED_TYPE_GENERIC`, `CRED_PERSIST_LOCAL_MACHINE`, `CredReadW`/`CredWriteW`). Windows persistence означает того же пользователя на той же машине, без roaming. Собственного master password SV нет; разблокировку системного хранилища обслуживает его provider.

Это явное исключение из shared-settings policy. Старый SDL3 не читает SV secrets; SV не записывает свой пароль в `tomenet.cfg`, `.tomenetrc`, INI, PRF или UI configuration. Существующий открытый пароль legacy config может оставаться: его cleanup вне SV. При записи общих настроек legacy password record сохраняется без замены значением SV; все startup/login/conversion/manual-save/shutdown writers должны соблюдать это исключение. Иные common settings сохраняют согласованное ownership.

Системное хранилище сохраняет точные credential bytes, без Unicode normalization и преобразования по font. Runtime/protocol `my_memfrob` не является шифрованием at rest и не определяет storage format; переходы между raw secret и core representation должны быть явными. Binary libsecret/Windows blob не навязывает UTF-8/UTF-16 secret payload. Metadata, labels, names и lookup keys не содержат секрет или его обратимое представление.

API evidence и ограничения: [Secure credential storage APIs](../research/secure-credential-storage.md). Требуется usable persistent Secret Service provider; одно наличие libsecret не доказывает защищённую конфигурацию provider. Нельзя обещать изоляцию от произвольного кода с доступом того же OS user. Существующая legacy plaintext copy этим решением не защищается.

### Identity, login и import

Record identity — отдельный SV namespace плюс server address, effective port и account. Разные aliases/DNS names/IP автоматически не объединяются; серверное разрешение адреса не является основанием отправлять пароль другой identity. Account соответствует baseline login name semantics, без blanket lowercase. Windows case-insensitive target names не должны сливать разные case-sensitive account identities; конкретная collision-free serialization/reference schema принадлежит [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md).

При обычном входе пароль загружается только для выбранной identity и доступен форме ввода. Успешный login автоматически создаёт/обновляет соответствующую защищённую запись. Автовход управляется отдельно от запоминания и по умолчанию выключен; имеющаяся запись сама по себе не включает автовход. Ошибка аутентификации возвращает ручной ввод и не удаляет сохранённую запись. Успешный вход с исправленным паролем обновляет её по тому же правилу.

Legacy password читается только при явном импорте с preview/conflict policy из [Define settings and migration boundary](05-define-settings-and-migration-boundary.md). Существующий SV secret имеет приоритет, заменить его импортом можно только явно. Импорт привязывает пароль к выбранным server/port/account и сохраняет bytes точно; preview не раскрывает содержимое секрета. Невозможность записать secret не выдаётся за успешный импорт этой группы. Внешний источник неизменен; уже общий legacy password также не удаляется.

Отсутствие SV record, её внешнее удаление, ошибка чтения или отказ разблокировки не запускают автоматический fallback к открытому паролю старого конфига. Legacy loader не должен предварительно подставлять его в login memory, обходя это правило.

### Failure и session lifecycle

При недоступном хранилище, отсутствии службы или отмене разблокировки доступен ручной вход на текущую сессию. Открытого disk fallback нет. Отсутствующая запись отличается от ошибки доступа; UI объясняет невозможность загрузки/сохранения, не объявляя ошибку «пароля нет». Ошибка запоминания не превращает успешный игровой login в ошибку: текущая сессия продолжается, пользователь видит «Пароль не сохранён». Автоматических повторяющихся unlock prompts нет.

Операции хранилища не блокируют SDL event/render loop. Их завершение связано с исходной identity и текущей login operation; поздний результат после отмены/смены аккаунта не выполняет неожиданный автовход и не попадает в другой record. Промежуточные secret buffers освобождаются/очищаются после использования, runtime copy живёт лишь столько, сколько нужно действующему login/relogin contract; дисковая diagnostic copy не создаётся.

Отдельную опцию «Забыть пароль», UI для удаления records и remember-disable switch не добавляем. Это не требует запрещать пользователю управление записями средствами ОС; отсутствие записи после такого действия обрабатывается обычным ручным входом. Автоматическое запоминание после успешного входа сохраняется.

### Немедленное сохранение при смене пароля

После локального подтверждения нового пароля и успешной постановки запроса смены на отправку (`Send_change_password == 1` в baseline) runtime password обновляется и сразу запускается запись нового пароля в защищённое хранилище соответствующего аккаунта. Сохранение не откладывается до следующего login, не ждёт server acknowledgement и не разбирает текст игрового сообщения как подтверждение. Отмена формы, несовпадение повторного ввода либо локальная ошибка отправки не являются завершённой сменой и не запускают эту замену.

Успех записи означает только сохранение нового значения локально. Если сервер отклонит запрос или соединение оборвётся до обработки, защищённая запись всё равно может содержать новый пароль; автоматического rollback по текстовому сообщению нет. При следующей ошибке входа действует ручной ввод, после успешной аутентификации запись обновляется. Если сама запись в vault не удалась, текущая сессия продолжается с явным сообщением о несохранённости; нельзя объявлять новый пароль сохранённым. Перед заменой запись намеренно не удаляется.

### Outputs и границы

Секрет, его runtime/protocol representation и содержимое auth/password-change payload не попадают в presentation events, логи, diagnostic capture, обычные exports и конфигурационные dumps. Password fields не раскрывают значение в штатном UI; import preview сообщает о наличии/конфликте, а не печатает пароль. Ограничение относится к самим секретным полям, не отменяя обычные игровые сообщения и существующие message/character/screenshot exports.

Q3 не вводит новый exporter: отдельная функция выгрузки профиля с паролями и формат зашифрованного переноса не входят в этот контракт. Исходно согласованный legacy import сохраняется. Если отдельный перенос секретов когда-либо появится в новом scope, прежнее условие явного выбора не означает разрешения включать их автоматически.

### Follow-through и evidence

- [Specify persistence ownership and UI configuration schema](25-specify-persistence-ownership-and-ui-configuration-schema.md): metadata/identity serialization и references, per-writer исключение SV password из shared files, import group commit/error semantics. Storage policy не выбирается заново.
- [Design parity evidence and acceptance](09-design-parity-evidence-and-acceptance.md): synthetic-secret проверки обеих платформ, successful login/change-password writes, server reject/disconnect после отправки, locked/unavailable/cancel/save-failure cases, responsive UI и late completion, independent identities/case-sensitive accounts, legacy fallback prohibition и отсутствие секретов в outputs. Особо проверить сохранность legacy password при любом common-config save.
- Platform packaging должно включать клиентские зависимости libsecret/GLib для выбранного backend; отсутствие доступного persistent provider обрабатывается по session-only policy. Windows headers/import library не заменяют реального runtime evidence; actual blob limits и exact credential bytes входят в проверки. Конкретные dependency closure и OS scenarios принадлежат acceptance, а не новому product decision.

Новых decision tickets не требуется: точные schema и acceptance вопросы уже имеют владельцев. Resource-budget fog не меняется.
