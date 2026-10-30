# B2-006 — исправления по пользовательскому visual review

2026-10-29. Исправления подготовлены для повторного просмотра пользователем.
B2 не объявлен принятым. Исходные замечания и предыдущие результаты сохранены
в [acceptance report](sv-stage-b2-acceptance.md#предварительные-замечания-пользователя--2026-10-29).
После последующего запуска исправлены также [blur, double click и Back](sv-b2-interactive-bugs.md).

## Результат по замечаниям

| № | Реализация |
|---|---|
| 1 | Default fullscreen сохранён и подтверждён обычным production запуском без `--fixture-window`. Оконные прогоны используются только для явно обозначенного сравнения geometry. |
| 2 | Обычная анимация logo меняет символы на `'`, `)`, `(`, `.` с шагом 100 мс. Reduced motion сохраняет static `^`, как reference. |
| 3 | Удалено уменьшение background до 640×320. Загружается исходное разрешение; bytes всех 19 PNG совпадают с upstream. Scaling для cover/движения остаётся частью композиции, промежуточной потери разрешения нет. |
| 4 | Chronicle получает события из девяти официальных HTTPS-источников EU/NA/APAC: deaths, legends, superuniques. Offline snapshot не используется. Цвет, дата, регион, wrapping и scroll представлены в UI. |
| 5 | Обновлены карточки, группировка, palette, rounded borders, hover/selection, host weight и keyboard hints. Точная метрика шрифта устраняет наложение региона на host. Direct перенесён из footer в header: все пять reference server slots помещаются при 1440×900. |
| 6 | Перенесена SVG-дверь reference с hover-состоянием. Клик по карточке выбирает сервер, клик по двери подключает к нему; keyboard actions сохранены. |
| 7 | Все шесть realm icons имеют строго круглую форму; SVG и PNG обновлены вместе с provenance и hashes. |

## Сетевая Chronicle

SV-local `input/chronicle.c/h` использует libcurl ≥7.85 и detached worker.
Проверка TLS включена; до девяти запросов выполняются параллельно, каждый body
ограничен 256 KiB, timeout — 12 секунд. Provider объединяет до 100 последних
событий; main owner получает согласованный snapshot и продолжает обслуживать input.
Отмена не ждёт сеть; provider живёт весь pregame flow, включая account/contact.
Loading, частичная недоступность и полный отказ источников различаются.

Production fetch подтвердил 9/9 успешных источников, по 40 событий на источник,
100 последних retained events и освобождение UI при cancel за 0 мс:
[live evidence](evidence/sv-b2-006-corrections/chronicle-live.txt),
[unavailable evidence](evidence/sv-b2-006-corrections/chronicle-unavailable.txt).
Это сетевые данные на момент запуска, а не обещание непрерывного push feed.

Используется официальный `https://www.tomenet.eu/legends.php?server=...` для всех
трёх регионов. Старые prototype URLs `legends-na.php` / `legends-apac.php`
вернули HTTP 403; canonical endpoint вернул HTTP 200 с региональными headings.
Содержимое upstream разных регионов может совпадать: клиент его не выдумывает.
Отдельная проблема web updater записана в [improvements](sv-improvements.md).

## Review и проверки

Два implementation agents разделили visual и network изменения; третий агент
выполнил независимое source/spec/visual review и Linux validation.
По review исправлены ранняя отмена provider, initial scroll при отсутствии
selection, наложение региона после bold, утечка font style в MOTD и footer,
скрывавший пятую карточку. Общие и legacy игровые модули не изменялись.

Команды, final executable fingerprints, фактические размеры screenshots,
результаты input и границы исторических captures приведены в
[independent review/validation](evidence/sv-b2-006-corrections/review-validation.md).
Минимальный production parser core check добавлен в существующий endpoint runner;
native-endpoint assertion адаптирован к раздельным row/door actions. Остальные
затронутые runners адаптированы только для сборки/link новой зависимости.
Новый runtime framework или постоянный expanded suite не добавлялся.

Software и OpenGL проверяются на Linux. Virtual geometry отделена от actual
fullscreen desktop; physical display timing, OS IME и Windows не заявляются.
Предшествующие B2 routes/ACK/MOTD evidence сохраняются исторически и не выдаются
за повторный полный запуск на каждом visual correction.

## Границы visual результата

В снимках этого прохода фоны стали резче после сохранения исходного разрешения.
Последующий [bug fix](sv-b2-interactive-bugs.md) добавляет явный blur и contrast
без уменьшения разрешения; прежние снимки сохраняются как история. Неизвестные
players/version/ping не заменены demo telemetry. Фактические события Chronicle
отличаются от старого browser snapshot. Внешние launchers остаются unavailable
по прежней границе C060; круглая форма иконок от этого не зависит.

Пользовательская visual acceptance ожидается; результат не объявляется
pixel-perfect только на основании прошедших tests.
