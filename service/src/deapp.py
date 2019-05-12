import sys, struct, hashlib
from Crypto.Cipher import AES
from Crypto.PublicKey import RSA

life = open(sys.argv[1]).read()
assert life[:4] == 'EFIL'
npages = struct.unpack('<I', life[4:8])[0]
print npages, 'pages'
print 'name: %s' % life[8:0x28]

KEYS = [
        open('root.key').read()[:0x20],
        'O' * 32,
        ]

CERTS = [
        RSA.importKey(open('system.priv').read()),
        RSA.importKey(open('platform.priv').read()),
        ]

cur = 0x28
raw = ''
for i in xrange(npages):
    base, size, prot, mode, key, cert = struct.unpack('<IIBBBB',
            life[cur: cur + 12])
    cur += 12
    print '%#x [%#x, %#x] %#x %#x' % (len(raw), base, base + size, prot, key)
    if not (prot & 0x8):
        raw += life[cur: cur + size].ljust(0x1000, '\x00')
        cur += size
        continue
    assert key == cert
    if prot & 0x10:
        # signed
        sig = life[cur: cur + 0x100]
        cur += 0x100
    else:
        sig = None
    if prot & 0x8:
        KEY = life[cur: cur + 0x20]
        IV = life[cur + 0x20: cur + 0x30]
        # assert key != 0 # root key is unknown
        if key <= 2:
            KEY = KEYS[key]
        print 'KEY: %s, IV: %s' % (KEY.encode('hex'), IV.encode('hex'))
        cur += 0x30
        assert mode <= 1
        cipher_text = life[cur: cur + size]
        if mode == 1:
            raw += AES.new(key=KEY, mode=AES.MODE_CBC,
                    IV=IV).decrypt(cipher_text).ljust(0x1000, '\x00')
        else:
            raw += AES.new(key=KEY, mode=AES.MODE_ECB).decrypt(cipher_text).ljust(0x1000, '\x00')
    if sig:
        # verify (no need)
        h = hashlib.sha1(cipher_text).digest()
        c = CERTS[key].encrypt(sig, 0)[0]
        if c.endswith(h):
            print 'valid sig'
        else:
            print 'invalid sig'
    elif prot & 4:
        print 'no sig'
    cur += size

if len(sys.argv) > 2:
    open(sys.argv[2], 'w').write(raw)
