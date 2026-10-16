# orc_camp: постановка только на open area

Дата: 2026-10-16.

Подтверждено отдельной диагностической сборкой текущего сервера (`generate`, `main`, `go` собраны с `-O0`, остальные объекты штатные), GDB остановлен перед `setup_contact_socket`; использована приватная копия `lib` в `/tmp/tomenet-orc-diagnosis`. Живой сервер, сохранения и настройки репозитория не изменены.

Orc Cave (тип 19), глубина 10, seed 1–8. Для обхода несвязанной player-only ветки случайных городов в изолированном экземпляре сняты только `DF2_TOWNS_RND|DF2_TOWNS_FIX`.

- Исходное заполнение `d_info[19].fill_type`: terrain 97 (mountain chain, PERMANENT). На семи обычных этажах 33 отказа builder, 768 отказов route, ноль постановок. Seed 6 создаёт open area: три успешных постановки.
- Контроль: изменены только пять `d_info[19].fill_type` на 56 (гранит), тот же процесс и те же seed. На каждом из семи обычных этажей появились шаблоны, всего 21 постановка. На open area те же три постановки.

Причина: `room_template_cell_is_connectable` запрещает маршруты через `FF1_PERMANENT`. Обычный этаж заполнен permanent-горами, поэтому от свободного блока шаблона нельзя дойти до существующей комнаты. На open area горного заполнения нет. Входы текущего orc_camp корректны; сам шаблон менять для устранения этой причины не нужно. Возможное изменение dungeon design: заменить базовое заполнение Orc Cave на terrain без PERMANENT. Снятие защиты permanent-клеток в алгоритме входов не применялось.

Диагностические скрипты, бинарный файл и полный лог временно сохранены в `/tmp/tomenet-orc-diagnosis/` (`probe.py`, `probe.gdb`, `output.log`). Команда прогона: `TOMENET_PATH=/tmp/tomenet-orc-diagnosis/lib timeout 55s gdb -q -batch -x /tmp/tomenet-orc-diagnosis/probe.gdb /tmp/tomenet-orc-diagnosis/tomenet.server`. Это диагностика normal generation, не исчерпывающая проверка всех глубин или типов этажей.
