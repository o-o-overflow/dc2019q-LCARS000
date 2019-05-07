import sys, struct
from Crypto.Cipher import AES
from Crypto.PublicKey import RSA

KEYS = [
        '\x00' * 32,
        'O' * 32,
        'A' * 32,
        ]

CERTS = [
        RSA.importKey(open('system.priv').read()),
        RSA.importKey(open('platform.priv').read()),
        ]

def pad(m):
    assert len(m) <= 252
    return '\x00\x01' + ('\x00' + m).rjust(254, '\xff')

def sign(c, m):
    return CERTS[int(c)].decrypt(pad(m))

class Page(object):
    def __init__(self, base, prot, raw='', key=-1):
        assert len(raw) <= 0x1000
        self.base = base
        self.raw = raw
        self.prot = prot
        self.key = key

    def encrypt(self, key):
        pass

    def __str__(self):
        if self.key == -1:
            data = self.raw
            prot = self.prot
        else:
            iv = open('/dev/urandom').read(16)
            K = KEYS[self.key]
            hdr = chr(1) + chr(self.key) + K + iv
            assert len(hdr) == 0x32
            while len(self.raw) % 0x10:
                self.raw += '\x00'
            data = hdr + AES.new(key=K, mode=AES.MODE_CBC,
                    IV=iv).encrypt(self.raw)
            prot = self.prot | 8
        return struct.pack('<III', self.base, len(self.raw), prot) + data

class App(object):
    def __init__(self, name='app'):
        self.name = name.ljust(0x20, '\x00')[:0x20]
        self.pages = []

    def add_segment(self, base, prot, raw, key=-1):
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
