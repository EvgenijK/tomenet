from pathlib import Path
from PIL import Image
import os,subprocess,tempfile,json
root=Path.cwd();out=root/'.scratch/sv-b2-006-corrections';records=[]
for frame in [120,128]:
 with tempfile.TemporaryDirectory(prefix='sv-b2-animation-') as profile:
  bmp=out/f'animated-2560x1600-frame{frame}.bmp';png=bmp.with_suffix('.png')
  env=dict(os.environ,SDL_VIDEODRIVER='x11',SDL_RENDER_DRIVER='software',SV_B2_BACKGROUND='0',SV_B2_REDUCED_MOTION='0',SV_B2_CAPTURE=str(bmp),SV_B2_CAPTURE_FRAME=str(frame))
  cmd=['/tmp/sv-b2-006-corrections-final','--endpoint','--server-list',str(out/'servers-five.txt'),'--library',str(root/'lib'),'--profile-root',profile,'--fixture-window','2560x1600','--frames',str(frame+2)]
  with (out/f'animation-{frame}.log').open('w') as log:subprocess.run(cmd,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=20)
  with Image.open(bmp) as im:im.save(png);size=im.size
  bmp.unlink();records.append(dict(command=cmd,output=list(size),filename=png.name,animation='ordinary enabled; independent processes target frame120/128'))
(out/'animation.json').write_text(json.dumps(records,indent=2))
