"""One-off X11 reproduction of B2-004 screenshots and visible input controls.
Uses the production executable and actual XTest input; contains no client logic.
Run from repo root, with DISPLAY and XAUTHORITY for the local Linux display.
"""
from pathlib import Path
import ctypes as C
import json
import os
import subprocess as S
import tempfile
import time

out = Path.cwd()/'.scratch/sv-b2-006-corrections'
out.mkdir(parents=True,exist_ok=True)
root = Path.cwd()
x = C.CDLL('libX11.so.6'); t = C.CDLL('libXtst.so.6')
ptr=C.c_void_p; ul=C.c_ulong; ui=C.c_uint; ii=C.c_int
handler_type=C.CFUNCTYPE(ii,ptr,ptr)
ignore_x_error=handler_type(lambda display,event:0)
x.XSetErrorHandler.argtypes=[handler_type]; x.XSetErrorHandler(ignore_x_error)
x.XOpenDisplay.restype=ptr; x.XOpenDisplay.argtypes=[C.c_char_p]
d=x.XOpenDisplay(None)
if not d: raise RuntimeError('XOpenDisplay failed')
x.XDefaultRootWindow.argtypes=[ptr]; x.XDefaultRootWindow.restype=ul
r=x.XDefaultRootWindow(d)
x.XSync.argtypes=[ptr,ii]
x.XkbGetState.argtypes=[ptr,ui,ptr]; x.XkbLockGroup.argtypes=[ptr,ui,ui]
kstate=C.create_string_buffer(32); x.XkbGetState(d,0x100,kstate); original_group=kstate.raw[1]
import atexit
atexit.register(lambda: (x.XkbLockGroup(d,0x100,original_group),x.XSync(d,0)))
x.XkbLockGroup(d,0x100,0); x.XSync(d,0)
x.XQueryTree.argtypes=[ptr,ul,C.POINTER(ul),C.POINTER(ul),C.POINTER(C.POINTER(ul)),C.POINTER(ui)]
x.XFetchName.argtypes=[ptr,ul,C.POINTER(C.c_char_p)]
x.XFree.argtypes=[ptr]
x.XSetInputFocus.argtypes=[ptr,ul,ii,ul]
x.XRaiseWindow.argtypes=[ptr,ul]
x.XMoveWindow.argtypes=[ptr,ul,ii,ii]
x.XSync.argtypes=[ptr,ii]
x.XKeysymToKeycode.argtypes=[ptr,ul]; x.XKeysymToKeycode.restype=ui
x.XStringToKeysym.argtypes=[C.c_char_p]; x.XStringToKeysym.restype=ul
t.XTestFakeKeyEvent.argtypes=[ptr,ui,ii,ul]
t.XTestFakeMotionEvent.argtypes=[ptr,ii,ii,ii,ul]
t.XTestFakeButtonEvent.argtypes=[ptr,ui,ii,ul]
x.XInternAtom.argtypes=[ptr,C.c_char_p,ii]; x.XInternAtom.restype=ul
pid_atom=x.XInternAtom(d,b'_NET_WM_PID',0)
x.XGetWindowProperty.argtypes=[ptr,ul,ul,C.c_long,C.c_long,ii,ul,C.POINTER(ul),C.POINTER(ii),C.POINTER(ul),C.POINTER(ul),C.POINTER(ptr)]
def owned(w):
    actual=ul(); fmt=ii(); count=ul(); remain=ul(); data=ptr()
    x.XGetWindowProperty(d,w,pid_atom,0,1,0,0,C.byref(actual),C.byref(fmt),C.byref(count),C.byref(remain),C.byref(data))
    result=bool(data and count.value and C.cast(data,C.POINTER(ul))[0]==desired_pid)
    if data:x.XFree(data)
    return result
def windows(w):
    name=C.c_char_p()
    if x.XFetchName(d,w,C.byref(name)) and name.value:
        value=name.value.decode(errors='replace'); x.XFree(name)
        if value=='TomeNET SV - Server' and owned(w): return w
    rr=ul(); pp=ul(); children=C.POINTER(ul)(); n=ui()
    if x.XQueryTree(d,w,C.byref(rr),C.byref(pp),C.byref(children),C.byref(n)):
        values=[children[i] for i in range(n.value)]
        if children: x.XFree(children)
        for child in values:
            match=windows(child)
            if match: return match
    return None
def key(name):
    code=x.XKeysymToKeycode(d,x.XStringToKeysym(name.encode()))
    t.XTestFakeKeyEvent(d,code,1,0); t.XTestFakeKeyEvent(d,code,0,0); x.XSync(d,0); time.sleep(.10)
def text(value):
    for ch in value: key(ch)
x.XTranslateCoordinates.argtypes=[ptr,ul,ul,ii,ii,C.POINTER(ii),C.POINTER(ii),C.POINTER(ul)]
x.XResizeWindow.argtypes=[ptr,ul,ui,ui]
def click(w,px,py):
    w=windows(r) or w
    rx=ii();ry=ii();child=ul()
    x.XTranslateCoordinates(d,w,r,round(px),round(py),C.byref(rx),C.byref(ry),C.byref(child))
    px=rx.value;py=ry.value
    t.XTestFakeMotionEvent(d,-1,round(px),round(py),0)
    t.XTestFakeButtonEvent(d,1,1,0);t.XTestFakeButtonEvent(d,1,0,0);x.XSync(d,0);time.sleep(.15)
def capture(w,name):
    time.sleep(.3)
    w=windows(r) or w
    S.run(['import','-window',hex(w),str(out/name)],check=True,timeout=5)


from PIL import Image
backend='software';scale=1;width=2560;height=1600
with tempfile.TemporaryDirectory(prefix='sv-b2-006-4k-') as profile:
 bmp=Path(profile)/'frame.bmp'
 env=dict(os.environ,SDL_VIDEODRIVER='x11',SDL_RENDER_DRIVER=backend,SDL_VIDEO_X11_SCALING_FACTOR='1',SV_B2_BACKGROUND='0',SV_B2_REDUCED_MOTION='0',SV_B2_CAPTURE=str(bmp),SV_B2_CAPTURE_FRAME='220')
 cmd=['/tmp/sv-b2-006-corrections-final','--endpoint','--server-list',str(out/'servers-five.txt'),'--library',str(root/'lib'),'--profile-root',profile,'--fixture-window','2560x1600']
 with (out/'software-2560-virtual.log').open('w') as stream:
  p=S.Popen(cmd,cwd=root,env=env,stdout=stream,stderr=S.STDOUT);desired_pid=p.pid
  try:
   for _ in range(200):
    w=windows(r)
    if w:break
    time.sleep(.03)
   time.sleep(1);w=windows(r) or w
   # Unmanage only this task's owned window, allowing virtual client geometry
   # larger than the current desktop. No monitor/global WM settings change.
   x.XUnmapWindow.argtypes=[ptr,ul];x.XMapWindow.argtypes=[ptr,ul]
   x.XReparentWindow.argtypes=[ptr,ul,ul,ii,ii]
   x.XChangeWindowAttributes.argtypes=[ptr,ul,ul,ptr]
   attrs=(C.c_ulong*20)();C.cast(C.byref(attrs,88),C.POINTER(ii))[0]=1
   x.XUnmapWindow(d,w);x.XChangeWindowAttributes(d,w,512,attrs);x.XReparentWindow(d,w,r,0,0);x.XMapWindow(d,w);x.XResizeWindow(d,w,2560,1600);x.XSync(d,0)
   for _ in range(500):
    if bmp.exists():break
    if p.poll() is not None:raise RuntimeError('client exited')
    time.sleep(.03)
   with Image.open(bmp) as im:output=im.size;im.save(out/'software-2560x1600-animation-virtual.png')
   record=dict(backend=backend,requestedLogical=[2560,1600],requestedOutput=[2560,1600],virtualDisplayScale=1,actualReadpixelsOutput=list(output),ownWindowOverrideRedirect=True,ownWindowReparentToRoot=True,command=cmd,physicalDisplayTiming=False)
   (out/'same-viewport-virtual.json').write_text(json.dumps(record,indent=2));print(record,flush=True)
  finally:
   if p.poll() is None:p.terminate()
   p.wait(timeout=5)
