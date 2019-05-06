from pwn import *

r = process(['./mon', './io.bin', './loader.bin', './echo.bin', './hello.bin'])
# r = process(['strace', '-f', './mon', './io.bin', './loader.bin', './echo.bin', './hello.bin'])

def download(rfile, lfile):
    with open(lfile) as f:
        blob = f.read()
        r.sendline('download %s %d' % (rfile, len(blob)))
        r.send(blob)

r.sendline('run echo.bin')
download('hello.app', 'hello.app')
r.sendline('run hello.app')

r.interactive()