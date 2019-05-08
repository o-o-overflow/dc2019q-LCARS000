import sys, struct, hashlib
from Crypto.Cipher import AES
from Crypto.PublicKey import RSA

KEYS = [
        open('root.key').read()[:0x20],
        'O' * 32,
        'A' * 32,
        ]

CERTS = [
        RSA.importKey(open('system.priv').read()),
        RSA.importKey(open('platform.priv').read()),
        ]

URANDOM = open('/dev/urandom')

def pad(m):
    assert len(m) <= 252
    return '\x00\x01' + ('\x00' + m).rjust(254, '\xff')

def sign(c, m):
    return CERTS[int(c)].decrypt(pad(m))

class Page(object):
    def __init__(self, base, prot, raw='', key=0xff):
        assert len(raw) <= 0x1000
        self.base = base
        self.raw = raw
        self.prot = prot
        self.key = key

    def encrypt(self, key):
        pass

    def __str__(self):
        if self.key == 0xff:
            data = self.raw
            prot = self.prot
        else:
            iv = URANDOM.read(16)
            K = KEYS[self.key]
            if self.key == 2:
                crypt_info = K + iv
            else:
                # do not leak the key
                crypt_info = URANDOM.read(32) + iv
            assert len(crypt_info) == 0x30
            while len(self.raw) % 0x10:
                self.raw += URANDOM.read(1)
            prot = self.prot | 8
            raw = AES.new(key=K, mode=AES.MODE_CBC, IV=iv).encrypt(self.raw)
            assert len(raw) == len(self.raw)
            if self.key < 2:
                prot = prot | 0x10
                h = hashlib.sha1(raw).digest()
                sig = sign(self.key, h)
                assert len(sig) == 0x100
            else:
                sig = ''
            data = sig + crypt_info + raw
        return struct.pack('<IIBBBB', self.base, len(self.raw), prot, 1,
                self.key, self.key) + data

class App(object):
    def __init__(self, name='app'):
        self.name = name.ljust(0x20, '\x00')[:0x20]
        self.pages = []

    def add_segment(self, base, prot, raw, key=0xff):
        for i in xrange(0, len(raw), 0x1000):
            self.pages.append(Page(base + i, prot, raw[i:i + 0x1000], key))

    def __str__(self):
        return ''.join([
            'EFIL',
            struct.pack('<I', len(self.pages)),
            self.name
            ] + map(str, self.pages))


if __name__ == '__main__':
    if len(sys.argv) == 3:
        certid, msg = sys.argv[1:3]
        sys.stdout.write(sign(certid, msg))
    elif len(sys.argv) >= 4:
        name, fin, key = sys.argv[1:5]
        a = App(name)
        a.add_segment(0x100000, 5, open(fin).read(), int(key))
        a.add_segment(0x200000, 3, '\x00' * 0x10)
        sys.stdout.write(str(a))
