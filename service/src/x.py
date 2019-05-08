from pwn import *

r = process(['./mon', './init.sys', './loader.sys', './echo.sys',
    './crypto.sys', './test.app', 'root.key'])
# r = process(['strace', '-f', './mon', './io.sys', './loader.sys', './echo.sys'])

def download(rfile, lfile):
    with open(lfile) as f:
        blob = f.read()
        r.sendline('download %s %d' % (rfile, len(blob)))
        r.send(blob)

# download('test.app', 'test.app')
# r.sendline('run test.app')
download('untrusted.app', 'untrusted.app')
download('platform.app', 'platform.app')
download('system.app', 'system.app')
# r.sendline('run untrusted.app')
# r.sendline('run platform.app')
r.sendline('run system.app')

r.interactive()
