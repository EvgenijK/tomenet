# Независимое source review — account followup, 2026-10-29

Точный delta сравнён с `/tmp/sv-b2-account-followup-baseline.tar`; перед чтением исходников выполнен Graphify query. CONTEXT и ownership/lifecycle ADR ранее прочитаны. Production-код reviewer не менял, новые tests/suites не добавлял.

Проверены shared roundrect/measure, анимация факелов, account scene/layout и guide branch в credentials input. Критических/функциональных замечаний пока нет: новые tip/flame glyph sets совпадают с source HTML, шаг использует shell frame100ms; reduced сохраняет статические символы; ember/handle/base роли раздельны. Measure использует тот же font/style/size, что paint; временный render scale guide восстанавливается. Ссылки содержат только публичный URL.

Password продолжает идти через `b2_field` без retained shell text cache; локальный draft обнуляется, содержимое секрета не попадает в новый cached measure/text. Полный фон поля не меняет его смысл: reveal hit обрабатывается перед password field и использует тот же shared form layout, Back/submit совпадают с painted rect. В credentials loop новые клики проходят прежний stale gesture timestamp guard. Focus lost выключает reveal. Capture по-прежнему запрещён при `credentials->reveal`, новый rendering его не обходит. Очистка после Back и generation lifecycle не изменены. Status nullable корректно обрабатывается `b2_status`; реальные errors/vault states сохранены.

Scope-замечание исправлено и повторно проверено: `b2_account_field` включает новую rounded/focus paint-role только для credentials; обычный `b2_field` сохраняет прежний paint для manual-address и character-name. Новый `f.account_back` используется account renderer, credentials input и existing scene fixture; прежний `f.back` connection Cancel не изменён. Show16 измеряется реальным font metric и right-align внутри shared hit rect; это соответствует итоговому CSS cascade, где button font inheritance перекрывает локальный fontsize10. Открытых замечаний нет.

`git diff --check` по пяти затронутым production файлам чистый. Финальную Linux сборку, existing endpoint suite и реальные captures/input выполняет root; этот отчёт не подменяет runtime evidence.

Final freeze повторно просмотрен после scope-изоляции и Show16 correction. No open source findings; root выполняет actual X11 и финальные existing checks.
