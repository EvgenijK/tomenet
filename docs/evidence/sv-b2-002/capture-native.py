"""One-off X11 reproduction of B2-002 screenshots and visible input controls.
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

out = Path(__file__).resolve().parent
root = out.parents[2]
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
def click(w,px,py):
    # Capture position is pinned to 0,0 in root coordinates.
    t.XTestFakeMotionEvent(d,-1,round(px),round(py),0)
    t.XTestFakeButtonEvent(d,1,1,0);t.XTestFakeButtonEvent(d,1,0,0);x.XSync(d,0);time.sleep(.15)
def capture(w,name):
    from PIL import Image
    for _ in range(300):
        if bmp.exists():
            try:
                with Image.open(bmp) as im: im.save(out/name)
                bmp.unlink()
                return
            except OSError: pass
        if p.poll() is not None:raise RuntimeError('Client stopped before capture')
        time.sleep(.05)
    raise RuntimeError('Renderer capture missing')
records=[]
for backend in ['software','opengl']:
 for width,height in [(1440,900),(1024,768)]:
  for state in os.environ.get('SV_CAPTURE_STATES','server,manual,account').split(','):
   with tempfile.TemporaryDirectory(prefix='sv-b2-002-native-') as profile:
    bmp=Path(profile)/'frame.bmp'
    env=dict(os.environ,SV_B2_CAPTURE=str(bmp),SV_B2_CAPTURE_FRAME='400',SDL_VIDEODRIVER='x11',SDL_RENDER_DRIVER=backend,SV_B2_BACKGROUND='0',SV_B2_REDUCED_MOTION='1')
    command=['/tmp/sv-b2-002-capture','--endpoint','--library',str(root/'lib'),'--profile-root',profile,'--fixture-window',f'{width}x{height}','--frames','1200']
    command += ['--server','127.0.0.1:1'] if state=='account' else ['--server-list',str(out/'servers.txt')]
    log=out/f'{backend}-{state}-{width}x{height}.log'
    with log.open('w') as stream:
     p=S.Popen(command,cwd=root,env=env,stdout=stream,stderr=S.STDOUT)
     desired_pid=p.pid
     try:
      w=None
      for _ in range(100):
       w=windows(r)
       if w: break
       if p.poll() is not None: raise RuntimeError(f'Client stopped: {log}')
       time.sleep(.05)
      if not w: raise RuntimeError('Client window missing')
      time.sleep(.8); x.XRaiseWindow(d,w); x.XMoveWindow(d,w,0,0); x.XSync(d,0); time.sleep(.4); t.XTestFakeMotionEvent(d,-1,round(width/2),round(height/2),0); x.XSetInputFocus(d,w,1,0); x.XkbLockGroup(d,0x100,0); x.XSync(d,0); time.sleep(.3)
      if state=='server': key('Down')
      if state=='manual': key('q')
      if state=='account': text('testaccount'); key('Tab'); text('sample')
      capture(w,f'{backend}-{state}-{width}x{height}.png')
      if state=='manual': key('Escape')
      if state=='account': key('Tab'); key('Tab'); key('Escape')
      records.append(dict(backend=backend,state=state,size=[width,height],command=command))
     finally:
      if p.poll() is None: p.terminate()
      p.wait(timeout=5)
(out/('native-capture-'+os.environ.get('SV_CAPTURE_STATES','all')+'.json')).write_text(json.dumps(records,indent=2))
