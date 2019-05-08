from pwn import *

r = process(['./mon', './init.sys', './loader.sys', './echo.sys',
    './crypto.sys', './hello.sys', './hello.app', 'root.key'])
# r = process(['strace', '-f', './mon', './io.sys', './loader.sys', './echo.sys', './hello.sys'])

def download(rfile, lfile):
    with open(lfile) as f:
        blob = f.read()
        r.sendline('download %s %d' % (rfile, len(blob)))
        r.send(blob)

# download('hello.app', 'hello.app')
# r.sendline('run hello.app')
download('untrusted.app', 'untrusted.app')
download('platform.app', 'platform.app')
download('system.app', 'system.app')
# r.sendline('run untrusted.app')
# r.sendline('run platform.app')
r.sendline('run system.app')

r.interactive()
