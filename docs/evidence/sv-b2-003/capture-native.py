"""One-off X11 reproduction of B2-003 screenshots and visible input controls.
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
    t.XTestFakeButtonEvent(d,1,1,0);t.XTestFakeButtonEvent(d,1,0,0);x.XSync(d,0);time.sleep(.15)
def capture(w,name):
    from PIL import Image
    for _ in range(300):
        if bmp.exists():
            time.sleep(.3)
            try:
                with Image.open(bmp) as im: im.save(out/name)
                bmp.unlink()
                S.run(["import","-window",str(w),str(out/name.replace(".png","-window.png"))],timeout=5,stdout=S.DEVNULL,stderr=S.DEVNULL)
                return
            except OSError: pass
        if p.poll() is not None:raise RuntimeError('Client stopped before capture')
        time.sleep(.05)
    raise RuntimeError('Renderer capture missing')

import socket,struct,threading
# The peer sends only real contact/setup/overview wire packets, following the
# existing sv_login_live_checks.py sequence. No renderer or behavior replica.
def exact(peer,count):
 data=b""
 while len(data)<count:
  part=peer.recv(count-len(data))
  if not part: raise RuntimeError("peer closed")
  data+=part
 return data
def field(peer):
 data=b""
 while not data.endswith(b"\0"):data+=exact(peer,1)
 return data
def serve(listener,ready,observed):
 try:
  peer,_=listener.accept()
  with peer:
   peer.settimeout(20)
   exact(peer,4);field(peer);exact(peer,3);field(peer);field(peer);exact(peer,26)
   peer.sendall(bytes([255,0])+struct.pack(">II6I",0,2,4,9,4,0,0,0))
   exact(peer,1);field(peer);field(peer);field(peer)
   peer.sendall(bytes([2,1,122,6])+struct.pack(">I",12345)+struct.pack(">IhBBBI",0,20,0,0,0,13))
   exact(peer,8)
   wire=bytes([162])+struct.pack(">IIII",(12 if int(os.environ.get("SV_CAPTURE_ROWS","10"))==10 else 16)|(1<<8)|(1<<16),0,0,0xc0)
   for i in range(int(os.environ.get("SV_CAPTURE_ROWS","10"))):
    wire+=bytes([12])+struct.pack(">h",i%2)+b"\xffW\0"+f"Hero{i}".encode()+b"\0"+struct.pack(">hhh",42-i,0,0)+b"at home\0"
   wire+=bytes([12])+struct.pack(">h",0)+b"\0\0"+struct.pack(">hhh",0,0,0)+b"\0"
   peer.sendall(wire);ready.set()
   # Drain keepalive/choice until client closes; record exact outgoing bytes.
   data=b""
   while True:
    part=peer.recv(1024)
    if not part:break
    data+=part
   observed["outgoing_hex"]=data.hex()
 except Exception as e:observed["peer_error"]=str(e)
# KDE/Wayland compositor text input uses its own layout; XkbLockGroup only
# changes Xwayland state. Preserve and restore the compositor's actual layout.
layout_api=["gdbus","call","--session","--dest","org.kde.keyboard","--object-path","/Layouts","--method"]
prior_layout=int(S.check_output(layout_api+["org.kde.KeyboardLayouts.getLayout"],text=True).split()[1].strip(",)"))
def layout(index):
 S.run(layout_api+["org.kde.KeyboardLayouts.setLayout",str(index)],check=True,stdout=S.DEVNULL);time.sleep(.15)
atexit.register(lambda:layout(prior_layout))
records=[]
for backend in os.environ.get("SV_CAPTURE_BACKENDS","software,opengl").split(","):
 for width,height in ([(1440,900)] if os.environ.get("SV_CAPTURE_ONE") else [(1440,900),(1024,768)]):
  for state in os.environ.get("SV_CAPTURE_STATES","overview,name,edit,error,scroll").split(","):
   with tempfile.TemporaryDirectory(prefix="sv-b2-003-native-") as profile, socket.socket() as listener:
    cfg=Path(profile)/"sv/tomenet.cfg";cfg.parent.mkdir();cfg.write_text("svSchemaVersion\t1\n")
    listener.bind(("127.0.0.1",0));listener.listen(1)
    ready=threading.Event(); observed={}
    worker=threading.Thread(target=serve,args=(listener,ready,observed),daemon=True);worker.start()
    bmp=Path(profile)/"frame.bmp"
    env=dict(os.environ,SV_B2_CAPTURE=str(bmp),SV_B2_CAPTURE_FRAME="180",SDL_VIDEODRIVER="x11",SDL_RENDER_DRIVER=backend,SV_B2_BACKGROUND="0",SV_B2_REDUCED_MOTION="1")
    command=[str(out/"tomenet-sv-capture"),"--endpoint","--server","127.0.0.1","--port",str(listener.getsockname()[1]),"--account","Test","--password-stdin","--library",str(root/"lib"),"--profile-root",profile,"--fixture-window",f"{width}x{height}"]
    log=out/f"{backend}-{state}-{width}x{height}.log"
    with log.open("w") as stream:
     p=S.Popen(command,cwd=root,env=env,stdin=S.PIPE,stdout=stream,stderr=S.STDOUT);p.stdin.write(b"pw\n");p.stdin.close();desired_pid=p.pid
     try:
      w=None
      for _ in range(100):
       w=windows(r)
       if w and ready.is_set():break
       if p.poll() is not None:raise RuntimeError(f"client stopped: {log}")
       time.sleep(.05)
      if not w or not ready.is_set():raise RuntimeError(f"window/overview missing: {log} {observed}")
      x.XRaiseWindow(d,w);x.XMoveWindow(d,w,0,0);x.XSetInputFocus(d,w,1,0);x.XkbLockGroup(d,0x100,0);x.XSync(d,0);layout(0);time.sleep(.25)
      if state in ["name","edit","error"]:
       code=x.XKeysymToKeycode(d,x.XStringToKeysym(b"Shift_L"));t.XTestFakeKeyEvent(d,code,1,0);key("n");t.XTestFakeKeyEvent(d,code,0,0);x.XSync(d,0)
      if state in ["edit","error"]:
       layout(0);text("Newhero")
      if state=="edit":
       # Production mouse double click selects the actual editable draft.
       px=450 if width==1440 else 390;py=450 if width==1440 else 330
       click(w,px,py);click(w,px,py)
      if state=="error":
       # The user's configured Cyrillic group produces non-Latin-1 text.
       layout(1);key("a");layout(0)
      if state in ["scroll","resize"]:
       for _ in range(int(os.environ.get("SV_CAPTURE_ROWS","10"))-1):key("Down")
       if state=="resize":x.XResizeWindow(d,w,1024,768);x.XSync(d,0);time.sleep(.25)
      capture(w,f"{backend}-{state}-{width}x{height}.png")
      records.append(dict(backend=backend,state=state,size=[width,height],command=command))
     finally:
      if p.poll() is None:p.terminate()
      p.wait(timeout=5);worker.join(timeout=2)
    records[-1]["peer"]=observed
(out/"native-capture.json").write_text(json.dumps(records,indent=2))
