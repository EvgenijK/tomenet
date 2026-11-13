# B2-006 — экран входа на сервер: исправление по двум скриншотам

2026-10-29. Исправления реализованы, code review и Linux проверки прошли;
пользовательская приёмка B2 ожидается. Исходное состояние текущей итерации:
`/tmp/sv-b2-account-followup-baseline.tar`. Накопленный Git diff не является
полным diff этой итерации.

## Исправления

- Поля Name/Password получили радиус 6 logical units и source neutral border;
  focus выделен отдельной рамкой. Пароль — цельное поле, Show/Hide является
  встроенным текстовым контролом, без заливки/разделителя. Из final CSS cascade
  взят фактический наследуемый размер Show 16px.
- Log in — прямоугольная game-action кнопка с тонкой рамкой, тёмным фоном,
  градиентом и uppercase-текстом 10px с tracking. Back — secondary, 58×32,
  с приглушённым текстом. Форма и hitboxes используют общие logical rectangles.
- Header получил отдельную подложку, нужный цвет/положение serif-заголовка
  и короткую затухающую линию после него; footer — свой тёмный фон.
- Под формой восстановлены дословные rules и текст рекомендации Guide из
  frozen HTML. URL подчёркнут, кликабелен и открывает официальный адрес.
  Убраны `Guide: unavailable` и постоянный default-текст про сохранение пароля;
  реальные ошибки, clipboard/encoding/lookup/vault statuses сохранены.
- Анимация факелов меняет символы верхушки из `')(.`, пламени из `0o*` и цвета
  каждые 100 ms. Ember остаётся `#`, меняя цвет. Reduced motion оставляет
  `^`, `*`, `#` статичными. Геометрия — bold 20px / line-height 17.2,
  inset соответствует account scene; высота и верхнее положение исправлены.
- Подложка account откалибрована отдельно от серверного списка. Фактический
  output sample body `(13,17,13)`, header `(24,25,20)` совпадает с выбранными
  областями присланного образца. Это калибровка native composition, а не
  заявление о воспроизведении browser backdrop-filter pixel-for-pixel.

## Что дополнительно нашёл агент

Независимый [анализ двух скриншотов](evidence/sv-b2-account-followup/visual-analysis.md)
выявил кроме общих замечаний слишком длинные/разреженные и прижатые к краям
факелы, чрезмерную ширину Back, неверный muted-role подписей Name/Password,
раздельный password/reveal block и отсутствие декора header. Эти предложения
включены в исправление. Разные серверы, live Chronicle, artwork и кадр logo
признаны различиями состояния, а не дефектами.

Для сравнения читаемости Chronicle на разных artwork агент предложил контроль
при одинаковом фоне/фазе анимации. По двум разным artwork непрозрачность/blur
Chronicle не менялись наугад. Полный анализ сохраняет это предложение отдельно.

## Ревью и проверка

[Source review](evidence/sv-b2-account-followup/source-review.md) проверил
input/hitboxes, privacy/capture suppression, torch source glyphs и scope.
Первое замечание касалось shared field/back изменений: исправление изолировано
в account, прежние manual/name fields и connection Cancel сохранены.
Открытых замечаний нет. Production scope: пять SV файлов; existing scene fixture
механически использует новый account Back rectangle. Legacy/common не менялись.

- `make -C src -f makefile.sv tomenet-sv -j4` — PASS.
- `python3 tests/sv_endpoint_checks.py` — PASS с новой геометрией и input.
  Последняя дальнейшая правка только подняла декоративные факелы на 8 units;
  после неё сборка и production render/input проверки повторены.
- Scratch probe вызывает actual `sv_login_shell_torches`, проверяя public glyph
  cache: baseline 0/6 dynamic symbols (RED), после 6/6 (GREEN). Reduced motion
  0/6 dynamic symbols ожидаемо, UBSan / strict-Werror PASS. Отдельного поведения
  факелов только для теста не создано; probe управляет phase production Shell.
- Production executable с временной SDL mouse/text injection при virtual DPI
  125%, OpenGL: выбор сервера → guide → ввод синтетических Probe/Nonsecret →
  Show → Hide → Back → список серверов → SDL Quit. OS URL boundary перехвачен,
  адрес проверен, реальная вкладка не открывается. Рендер текстур подтвердил
  `Show → Hide → Show`; Back вернул список без закрытия процесса.
- Финальные software output 2560×1600 / logical 1920×1200 и OpenGL
  1280×960 / logical 1024×768 проверены визуально. DPI виртуальный, физические
  клики и physical display timing этой проверкой не заявляются.
- Source whitespace check PASS; Graphify обновлён. Новых постоянных test suites
  и frameworks нет; временные diagnostic probes остаются в `.scratch/`.

[Software после исправления](evidence/sv-b2-account-followup/final-software.png),
[OpenGL compact](evidence/sv-b2-account-followup/final-opengl-small.png),
[Back после проверки действий](evidence/sv-b2-account-followup/actions-back.png).
Основные design captures сняты с reduced motion для сравнения геометрии;
они не являются кадрами включённой анимации. Пустые поля/различие фокуса
следует учитывать при сравнении с браузерным скриншотом.

[Fingerprint](evidence/sv-b2-account-followup/binary.sha256),
[исходный образец](evidence/sv-b2-account-followup/user-reference.png),
[исходный клиент](evidence/sv-b2-account-followup/user-client-before.png).
