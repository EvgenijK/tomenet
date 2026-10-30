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
    rx=ii();ry=ii();child=ul()
    x.XTranslateCoordinates(d,w,r,round(px),round(py),C.byref(rx),C.byref(ry),C.byref(child))
    px=rx.value;py=ry.value
    t.XTestFakeMotionEvent(d,-1,round(px),round(py),0)
    x.XSync(d,0);time.sleep(.2)
    t.XTestFakeButtonEvent(d,1,1,0);t.XTestFakeButtonEvent(d,1,0,0);x.XSync(d,0);time.sleep(.15)
def capture(w,name):
    time.sleep(.3)
    S.run(['import','-window',hex(w),str(out/name)],check=True,timeout=15)


from PIL import Image
records=[]
for backend,width,height,scale,fullscreen in [('software',2560,1600,1,True),('software',1440,900,1,False),('opengl',1024,768,1.5,False)]:
 label=f'{backend}-{width}x{height}-scale{scale}'+('-fullscreen' if fullscreen else '')
 with tempfile.TemporaryDirectory(prefix='sv-b2-corrections-') as profile:
  env=dict(os.environ,SDL_VIDEODRIVER='x11',SDL_RENDER_DRIVER=backend,SDL_VIDEO_X11_SCALING_FACTOR=str(scale),SV_B2_BACKGROUND='0',SV_B2_REDUCED_MOTION='1')
  cmd=['/tmp/sv-b2-006-corrections-final','--endpoint','--server-list',str(out/'servers-five.txt'),'--library',str(root/'lib'),'--profile-root',profile]
  if not fullscreen:cmd+=['--fixture-window',f'{width}x{height}']
  with (out/(label+'.log')).open('w') as stream:
   p=S.Popen(cmd,cwd=root,env=env,stdout=stream,stderr=S.STDOUT);desired_pid=p.pid
   try:
    w=None
    for _ in range(200):
     w=windows(r)
     if w:break
     if p.poll() is not None:raise RuntimeError('client stopped')
     time.sleep(.03)
    if not w:raise RuntimeError('no window')
    if not fullscreen:x.XMoveWindow(d,w,0,0)
    x.XRaiseWindow(d,w);x.XSetInputFocus(d,w,1,0);x.XSync(d,0)
    time.sleep(2)
    w=windows(r)
    if not w:raise RuntimeError("startup final window not found")
    if not fullscreen:x.XMoveWindow(d,w,0,0)
    x.XRaiseWindow(d,w);x.XSetInputFocus(d,w,1,0);x.XSync(d,0)
    time.sleep(.2)
    capture(w,label+'-server.png')
    if fullscreen:
     width,height=Image.open(out/(label+'-server.png')).size
    state_atom=x.XInternAtom(d,b'_NET_WM_STATE',0)
    fullscreen_atom=x.XInternAtom(d,b'_NET_WM_STATE_FULLSCREEN',0)
    actual=ul();fmt=ii();count=ul();remain=ul();data=ptr()
    x.XGetWindowProperty(d,w,state_atom,0,32,0,0,C.byref(actual),C.byref(fmt),C.byref(count),C.byref(remain),C.byref(data))
    states=[C.cast(data,C.POINTER(ul))[i] for i in range(count.value)] if data else []
    if data:x.XFree(data)
    if fullscreen:assert fullscreen_atom in states,'production default not actual fullscreen'
    # Actual new card geometry: first official group starts 30 units below list.
    pw=min(860,max(400,width-414));px=(width-pw)/2
    hero=min(160,width*.1125+21) if width<=1240 else (190 if height<=800 else 250)
    py=(8 if height<=800 else 12)+hero+(8 if height<=800 else 10)
    listy=py+(51 if height<=800 else 58);rh=72 if height<=800 else 88
    row1y=listy+30+rh+9
    click(w,(px+90)*scale,(row1y+rh/2)*scale)
    capture(w,label+'-row-select.png')
    click(w,(px+pw-51)*scale,(row1y+rh/2)*scale)
    time.sleep(.5);capture(w,label+'-door-account.png')
    records.append(dict(label=label,command=cmd,backend=backend,logical=[width,height],virtualDisplayScale=scale,actualOutput=list(Image.open(out/(label+'-server.png')).size),actualFullscreen=fullscreen_atom in states,input='XTest second card select, same card door connect; screenshots inspected independently'))
   finally:
    if p.poll() is None:p.terminate()
    p.wait(timeout=5)
 (out/'runtime-input.json').write_text(json.dumps(records,indent=2))
print(json.dumps(records,indent=2))
