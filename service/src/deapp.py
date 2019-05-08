import sys, struct
from Crypto.Cipher import AES

life = open(sys.argv[1]).read()
assert life[:4] == 'EFIL'
npages = struct.unpack('<I', life[4:8])[0]
print npages, 'pages'
print 'name: %s' % life[8:0x28]

KEYS = [
        open('root.key').read()[:0x20],
        'O' * 32,
        ]

cur = 0x28
raw = ''
for i in xrange(npages):
    base, size, prot, mode, key, cert = struct.unpack('<IIBBBB',
            life[cur: cur + 12])
    cur += 12
    print '[%#x, %#x] %#x %#x' % (base, base + size, prot, key)
    assert mode == 1 # CBC
    assert key == cert
    if prot & 0x10:
        # signed
        cur += 0x100
    assert prot & 0x8
    KEY = life[cur: cur + 0x20]
    IV = life[cur + 0x20: cur + 0x30]
    print 'KEY: %s, IV: %s' % (KEY.encode('hex'), IV.encode('hex'))
    cur += 0x30
    if key < 2:
        KEY = KEYS[key]
    raw += AES.new(key=KEY, mode=AES.MODE_CBC, IV=IV).decrypt(life[cur: cur
        + size])
    cur += size
    break

open(sys.argv[2], 'w').write(raw)
