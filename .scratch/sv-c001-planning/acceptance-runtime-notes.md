# C001 runtime notes — 2026-11-04

Source applicability, не PASS. Canonical evidence/status меняет ведущий агент.

## Unavailable profile

Current production `src/client/sv/login/native-run.c:445-480` выбирает explicit
root, SDL3 override либо default `SDL_GetPrefPath`; `profile_first_run:49-57`
только проверяет `<root>/sv/tomenet.cfg`. `sv_font_open` в `ui/font.c:40-68`
пробует user TTF → bundled TTF → bundled PCF; failure одного overlay закономерно
даёт диагностическое сообщение и fallback, а не обязательный fatal.
`login/flow.c:629` create не пишет профиль; login identity вызывается только
после выбора endpoint в flow_begin. До login нет private profile write/replace.

Поэтому root-as-file/недоступный overlay + valid bundle **может корректно
запуститься**. Наблюдать fallback и неизменность sentinel file; не изобретать
ошибку startup или profile creation ради этого сценария. Это подтверждает
handling unavailable overlay, но не writable profile, C002 bootstrap, C003 file
operations, import или concurrent edit. Если default `SDL_GetPrefPath` вернул
NULL, startup failure применим; если он сам создал directory, это штатный SDL
bootstrap, не запись SV CFG. Fatal обязательных ресурсов проверять отказом всех
применимых bundled fallbacks. Отказ необязательного provider не равен fatal.

## Existing short runtime

Для извлечённого relocated artifact вне checkout доступны existing `--frames 2`
и `SV_B2_CAPTURE=/ABS/TEMP/startup.bmp`. Не задавать `--endpoint`, `--library`
или `TOMENET_PATH` для основного package lookup. Это короткий bounded production
run, но не заменяет отдельный bare invocation + штатный close. HOME/XDG paths
и root должны быть временными; не использовать реальные accounts/passwords.

## Одноразовый WM close для X11 bare invocation

Ниже заготовка, **не запускалась**; не новый suite и не installed tool.
Предварительно запустить extracted binary без аргументов в X11
(`SDL_VIDEO_DRIVER=x11`, temporary HOME/XDG/profile), сохранить PID. Команда
использует установленный `xprop` и libX11, выбирает только окно этого PID;
`WM_DELETE_WINDOW` проходит обычный SDL close path. Вызов Python: передать PID
argv[1]. Завершение процесса и его exit code проверять отдельно. Для Wayland
не применять: выбрать согласованный X11 runtime либо закрыть окно штатно.

```python
import ctypes as C
import ctypes.util
import re
import subprocess
import sys

pid = int(sys.argv[1])
root = subprocess.check_output(['xprop', '-root', '_NET_CLIENT_LIST'], text=True)
window = None
for candidate in re.findall(r'0x[0-9a-fA-F]+', root):
    info = subprocess.run(['xprop', '-id', candidate, '_NET_WM_PID'],
                          capture_output=True, text=True, check=True).stdout
    if re.search(r'=\s*' + str(pid) + r'\s*$', info):
        window = int(candidate, 16)
        break
if window is None:
    raise SystemExit('No X11 window for expected extracted process PID')

class Data(C.Union):
    _fields_ = [('b', C.c_char * 20), ('s', C.c_short * 10), ('l', C.c_long * 5)]
class ClientMessage(C.Structure):
    _fields_ = [('type', C.c_int), ('serial', C.c_ulong), ('send_event', C.c_int),
                ('display', C.c_void_p), ('window', C.c_ulong),
                ('message_type', C.c_ulong), ('format', C.c_int), ('data', Data)]
class Event(C.Union):
    _fields_ = [('xclient', ClientMessage), ('pad', C.c_long * 24)]

x = C.CDLL(ctypes.util.find_library('X11'))
x.XOpenDisplay.argtypes = [C.c_char_p]
x.XOpenDisplay.restype = C.c_void_p
x.XInternAtom.argtypes = [C.c_void_p, C.c_char_p, C.c_int]
x.XInternAtom.restype = C.c_ulong
x.XSendEvent.argtypes = [C.c_void_p, C.c_ulong, C.c_int, C.c_long, C.POINTER(Event)]
x.XFlush.argtypes = [C.c_void_p]
x.XCloseDisplay.argtypes = [C.c_void_p]
display = x.XOpenDisplay(None)
if not display:
    raise SystemExit('XOpenDisplay failed')
event = Event()
event.xclient.type = 33  # ClientMessage
event.xclient.display = display
event.xclient.window = window
event.xclient.message_type = x.XInternAtom(display, b'WM_PROTOCOLS', False)
event.xclient.format = 32
event.xclient.data.l[0] = x.XInternAtom(display, b'WM_DELETE_WINDOW', False)
event.xclient.data.l[1] = 0  # CurrentTime
if not x.XSendEvent(display, window, False, 0, C.byref(event)):
    raise SystemExit('XSendEvent failed')
x.XFlush(display)
x.XCloseDisplay(display)
```
