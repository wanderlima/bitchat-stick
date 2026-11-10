"""Temporary NVS reboot test; always attempts to reinstall production firmware."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import time
import serial
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--port',required=True,help='Explicit Plus2 serial port')
parser.add_argument('--pio',default=shutil.which('pio') or str(Path(sys.executable).parent/'pio'))
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]
pio=args.pio
port=args.port
def upload(env):
    result=subprocess.run([pio,'run','-e',env,'-t','upload','--upload-port',port],cwd=root,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
    print('\n'.join(result.stdout.splitlines()[-12:]),flush=True)
    result.check_returncode()
def read_boot():
    stream=serial.Serial(port=None,baudrate=115200,timeout=.2)
    stream.dtr=False;stream.rts=False;stream.port=port;stream.open()
    data=bytearray();deadline=time.monotonic()+8
    with stream:
        while time.monotonic()<deadline:data.extend(stream.read(4096))
    output=data.decode('utf-8',errors='replace')
    print(output,flush=True)
    return output
try:
    upload('settings-selftest')
    result=read_boot()
    assert 'SETTINGS_SELFTEST PASS' in result and 'SETTINGS_SELFTEST FAIL' not in result
finally:
    print('Restoring production firmware',flush=True)
    upload('m5stickc-plus2')
