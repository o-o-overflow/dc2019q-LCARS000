from pwn import *
import hashlib

r = process(['./mon', './init.sys', './loader.sys', './echo.sys',
    './crypto.sys', './svc.uapp', 'root.key', 'flag1.papp'])

def download(app):
    with open(app) as f:
        blob = f.read()
        r.sendline('download %s %d' % (app, len(blob)))
        r.send(blob)

download('exp1.uapp')
r.sendline('run exp1.uapp')
print r.recvuntil('BEGIN\n')
d = r.recvuntil('END\n', drop=True)
print hashlib.md5(d).hexdigest()

r.interactive()
