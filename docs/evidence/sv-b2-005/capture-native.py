"""One-off X11 reproduction of B2-005 screenshots and visible input controls.
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

out = Path(os.environ['SV_CAPTURE_OUTPUT']) if os.environ.get('SV_CAPTURE_OUTPUT') else Path(__file__).resolve().parent
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
    t.XTestFakeButtonEvent(d,1,1,0);t.XTestFakeButtonEvent(d,1,0,0);x.XSync(d,0);time.sleep(.15)
def capture(w,name):
    time.sleep(.3)
    S.run(['import','-window',str(w),str(out/name)],check=True,timeout=5)

import socket,struct,threading

def exact(peer,count):
 data=b''
 while len(data)<count:
  part=peer.recv(count-len(data))
  if not part:raise RuntimeError('peer closed')
  data+=part
 return data

def field(peer):
 data=b''
 while not data.endswith(b'\0'):data+=exact(peer,1)
 return data

# The existing B2-004 one-off replay is adapted only to capture B2-005 states.
def motd_payload(name):
 rows=json.loads((out/'fixtures.json').read_text())[name]
 result=b''
 for row in rows:
  data=bytearray(); color='w'
  for cell in row:
   cell=cell or {'ch':' ','color':'w'}
   if cell['color']!=color:
    color=cell['color'];data+=b'\xff'+color.encode()
   data+=cell['ch'].encode('ascii')
  if len(data)>119:raise RuntimeError((name,len(data)))
  result+=bytes(data).ljust(120,b'\0')
 return result

def setup(name):
 rows=b''
 for title in ['Human','Elf','Dwarf']:rows+=bytes([50])*6+title.encode()+b'\0'+struct.pack('>I',7)
 for title in ['Warrior','Ranger','Mage']:rows+=bytes([50])*6+title.encode()+b'\0'+bytes(6)
 for index,title in enumerate(['N/A','Dragon','Phoenix']):rows+=title.encode()+b'\0'+struct.pack('>I',7 if index else 0)
 motd=motd_payload(name)
 return struct.pack('>IhBBBI',len(motd),20,3,3,3,13+len(rows)+len(motd))+rows+motd

def serve(listener,ready,observed,advance,fixture):
 try:
  peer,_=listener.accept()
  with peer:
   peer.settimeout(60)
   exact(peer,4);field(peer);exact(peer,3);field(peer);field(peer);exact(peer,26)
   peer.sendall(bytes([255,0])+struct.pack('>II6I',0,2,4,9,4,0,0,0))
   exact(peer,1);field(peer);field(peer);field(peer)
   peer.sendall(bytes([2,1,122,6])+struct.pack('>I',12345)+setup(fixture))
   exact(peer,8)
   peer.sendall(bytes([162])+struct.pack('>IIII',2|(1<<8)|(1<<16),0,0,0xc0)+bytes([12])+struct.pack('>h',0)+b'\0\0'+struct.pack('>hhh',0,0,0)+b'\0')
   ready.set(); observed['phase']='overview'
   observed['name_request_hex']=(exact(peer,1)+field(peer)).hex();peer.sendall(b'\0');observed['phase']='birth'
   request=exact(peer,16266);observed['final_request_size']=len(request);observed['phase']='waiting'
   (out/(label+'.request')).write_bytes(request)
   if not advance.wait(15):raise RuntimeError('missing partial trigger')
   advance.clear();peer.sendall(b'\x02\x03');observed['phase']='partial'
   if not advance.wait(15):raise RuntimeError('missing ACK trigger')
   advance.clear();peer.sendall(b'\x7a');observed['phase']='ack'
   observed['ack_hex']='02 03 7a'
   while peer.recv(4096):pass
 except Exception as e:observed['peer_error']=str(e)

layout_api=['gdbus','call','--session','--dest','org.kde.keyboard','--object-path','/Layouts','--method']
prior_layout=int(S.check_output(layout_api+['org.kde.KeyboardLayouts.getLayout'],text=True).split()[1].strip(',)'))
def layout(index):
 S.run(layout_api+['org.kde.KeyboardLayouts.setLayout',str(index)],check=True,stdout=S.DEVNULL);time.sleep(.15)
atexit.register(lambda:layout(prior_layout))
def wait_phase(observed,phase):
 for _ in range(200):
  if observed.get('phase')==phase:return
  if 'peer_error' in observed:raise RuntimeError(observed)
  time.sleep(.025)
 raise RuntimeError(('phase timeout',phase,observed))
records=[]
for backend in os.environ.get('SV_CAPTURE_BACKENDS','software,opengl').split(','):
 for fixture,width,height in [('short',1024,768),('wide',1024,768),('notice',1440,900),('unknown',1440,900),('empty',1024,768)]:
  with tempfile.TemporaryDirectory(prefix='sv-b2-005-native-') as profile,socket.socket() as listener:
   cfg=Path(profile)/'sv/tomenet.cfg';cfg.parent.mkdir();cfg.write_text('svSchemaVersion\t1\n')
   listener.bind(('127.0.0.1',0));listener.listen(1)
   ready=threading.Event();advance=threading.Event();observed={}
   label=f'{backend}-{fixture}-{width}x{height}'
   worker=threading.Thread(target=serve,args=(listener,ready,observed,advance,fixture),daemon=True);worker.start()
   env=dict(os.environ,SDL_VIDEODRIVER='x11',SDL_RENDER_DRIVER=backend,SV_B2_BACKGROUND='0',SV_B2_REDUCED_MOTION='1',DBUS_SESSION_BUS_ADDRESS='unix:path=/tmp/sv-b2-005-no-service')
   command=[os.environ['SV_CAPTURE_BINARY'],'--endpoint','--server','127.0.0.1','--port',str(listener.getsockname()[1]),'--account','Test','--password-stdin','--library',str(root/'lib'),'--profile-root',profile,'--fixture-window',f'{width}x{height}']
   with (out/(label+'.log')).open('w') as stream:
    p=S.Popen(command,cwd=root,env=env,stdin=S.PIPE,stdout=stream,stderr=S.STDOUT);p.stdin.write(b'pw\n');p.stdin.close();desired_pid=p.pid
    try:
     w=None
     for _ in range(400):
      w=windows(r)
      if w and ready.is_set():break
      if p.poll() is not None:raise RuntimeError(f'client stopped: {label}')
      time.sleep(.05)
     if not w or not ready.is_set():raise RuntimeError(('window missing',observed,w))
     x.XRaiseWindow(d,w);x.XMoveWindow(d,w,0,0);x.XSetInputFocus(d,w,1,0);x.XSync(d,0);layout(0);time.sleep(.5)
     click(w,478 if width==1440 else 395,854 if width==1440 else 680);time.sleep(.3)
     for attempt in range(3):
      if attempt:click(w,478 if width==1440 else 395,854 if width==1440 else 680);time.sleep(.3)
      code=x.XKeysymToKeycode(d,x.XStringToKeysym(b'Control_L'));t.XTestFakeKeyEvent(d,code,1,0);key('a');t.XTestFakeKeyEvent(d,code,0,0);x.XSync(d,0)
      text('carol');key('Return');time.sleep(.3)
      if 'name_request_hex' in observed:break
     wait_phase(observed,'birth')
     key('Return');key('c');key('Return');key('c');key('Return');key('b');key('Return');key('f');key('Return')
     for ch in 'abcdef':key(ch);key('Return')
     if fixture=='notice':
      code=x.XKeysymToKeycode(d,x.XStringToKeysym(b'Shift_L'));t.XTestFakeKeyEvent(d,code,1,0);key('slash');t.XTestFakeKeyEvent(d,code,0,0);x.XSync(d,0)
      capture(w,f'{backend}-guide-{width}x{height}.png');key('Escape')
     key('Return');wait_phase(observed,'waiting')
     if fixture=='notice':capture(w,f'{backend}-waiting-{width}x{height}.png')
     advance.set();wait_phase(observed,'partial')
     if fixture=='notice':capture(w,f'{backend}-partial-{width}x{height}.png')
     advance.set();wait_phase(observed,'ack');capture(w,label+'.png')
     if fixture=='unknown':
      x.XResizeWindow(d,w,1024,768);x.XSync(d,0);capture(w,f'{backend}-unknown-resize-1024x768.png')
     key('a')
     if fixture=='notice':capture(w,f'{backend}-handoff-{width}x{height}.png')
     records.append(dict(backend=backend,size=[width,height],fixture=fixture,command=command,peer=observed))
     print(label,observed,flush=True)
    finally:
     if p.poll() is None:p.terminate()
     try:p.wait(timeout=5)
     except S.TimeoutExpired:p.kill();p.wait(timeout=5)
     worker.join(timeout=2)
   (out/'native-capture.json').write_text(json.dumps(records,indent=2))
