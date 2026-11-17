#!/usr/bin/env python3
"""Render the actual firmware composer with simulated data; no device or radio used."""
from pathlib import Path
import json, re, subprocess, tempfile

root = Path(__file__).resolve().parents[3]
output = root / 'website/public/screens'
crypto = root / '.pio/libdeps/m5stickc-plus2/Monocypher/src'
with tempfile.TemporaryDirectory(prefix='bitchat-demo-') as directory:
    temp = Path(directory)
    source = (root / 'test/ui_preview.cpp').read_text().replace('#include "../src/Ui.cpp"', f'#include "{root}/src/Ui.cpp"')
    extra = r'''
  privateDraft=true;queuedMessage=0;sendFailed=false;
  for(size_t method=0;method<3;++method){
    composer.open(other.id(),"Test phone");composer.preset(0);screen=Screen::Reply;selection=method;
    saveState(("reply-method-"+std::to_string(method)).c_str());
  }
  composer.open(other.id(),"Test phone");composer.preset(0);screen=Screen::Keyboard;selection=0;saveState("keyboard-sample");
  for(size_t index=0;index<MessageComposer::presetCount;++index){
    composer.open(other.id(),"Test phone");composer.preset(index);queuedMessage=0;sendFailed=false;reviewPage=0;
    screen=Screen::ShortMessages;selection=index;saveState(("short-"+std::to_string(index)).c_str());
    screen=Screen::Confirm;selection=reviewPages()+1;saveState(("review-"+std::to_string(index)+"-cancel").c_str());
    selection=reviewPages();saveState(("review-"+std::to_string(index)+"-send").c_str());
    queuedMessage=9999;selection=0;saveState(("queued-"+std::to_string(index)).c_str());
  }
'''
    (temp/'preview.cpp').write_text(source[:source.rfind('}')]+extra+'\n}\n')
    for src, name in [(crypto/'monocypher.c','monocypher'), (crypto/'optional/monocypher-ed25519.c','ed25519'), (root/'.pio/libdeps/m5stickc-plus2/M5GFX/src/lgfx/utility/lgfx_miniz.c','miniz')]:
        subprocess.run(['cc','-std=c99','-I',str(crypto),'-c',str(src),'-o',str(temp/f'{name}.o')], check=True)
    sources=['Feedback','PeerTable','PeerSelection','Settings','Mesh','Identity','NoiseSession','FragmentAssembler','WireCodec','PacketSecurity','CryptoPrimitives']
    subprocess.run(['c++','-std=c++17','-Wall','-Wextra','-Werror','-DSTICK_BOARD_M5STICKC_PLUS2','-I',str(root/'test/fakes'),'-I',str(root/'include'),'-I',str(root/'src'),'-I',str(crypto),str(temp/'preview.cpp'),str(root/'test/fakes/HostBoard.cpp'),*[str(root/f'src/{name}.cpp') for name in sources],*[str(temp/f'{name}.o') for name in ['monocypher','ed25519','miniz']],'-o',str(temp/'preview')],check=True,cwd=root)
    subprocess.run([str(temp/'preview'),str(temp)],check=True,cwd=root)
    # Use the same firmware font renderer at native resolution, not enlarged 4x.
    renderer=(root/'tools/render-previews.py').read_text().replace('root=Path(__file__).resolve().parents[1]',f'root=Path({str(root)!r})')
    a=renderer.index(' rows=[]');b=renderer.index("print('FreeMono",a)
    renderer=renderer[:a]+" png(source.with_suffix('.png'),bytes(pixels),135,240)\n"+renderer[b:]
    (temp/'render.py').write_text(renderer)
    subprocess.run(['python3',str(temp/'render.py'),str(temp)],check=True)
    files=[p for p in temp.glob('*.png') if p.stem.startswith(('short-','review-','queued-','reply-method-','keyboard-sample'))]
    for file in files:(output/file.name).write_bytes(file.read_bytes())
    header=(root/'src/MessageComposer.h').read_text().split('presets[]={',1)[1].split('};',1)[0]
    presets=re.findall(r'"([^"]*)"',header)
    (root/'website/src/data/short-messages.json').write_text(json.dumps(presets,indent=2)+'\n')
    print(f'{len(files)} native-resolution screenshots; {len(presets)} presets exported from firmware.')
