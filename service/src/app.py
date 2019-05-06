import sys, struct
from Crypto.Cipher import AES

ROOT_KEY = '\x00' * 32
PROVISION_KEY = 'O' * 32

KEYS = {
        ROOT_KEY: 0,
        PROVISION_KEY: 1
        }

class Page(object):
    def __init__(self, base, prot, raw='', key=None):
        assert len(raw) <= 0x1000
        self.base = base
        self.raw = raw
        self.prot = prot
        self.key = key

    def encrypt(self, key):
        pass

    def __str__(self):
        if self.key is None:
            data = self.raw
            prot = self.prot
        else:
            iv = open('/dev/urandom').read(16)
            key_id = KEYS.get(self.key, 2)
            key = '\x00' * 32 if key_id != 2 else self.key
            hdr = chr(1) + chr(key_id) + key + iv
            assert len(hdr) == 0x32
            while len(self.raw) % 0x10:
                self.raw += '\x00'
            data = hdr + AES.new(key=self.key, mode=AES.MODE_CBC,
                    IV=iv).encrypt(self.raw)
            prot = self.prot | 8
        return struct.pack('<III', self.base, len(self.raw), prot) + data

class App(object):
    def __init__(self, name='app'):
        self.name = name.ljust(0x20, '\x00')[:0x20]
        self.pages = []

    def add_segment(self, base, prot, raw, key=None):
        for i in xrange(0, len(raw), 0x1000):
            self.pages.append(Page(base + i, prot, raw[i:i + 0x1000], key))

    def __str__(self):
        return ''.join([
            'EFIL',
            struct.pack('<I', len(self.pages)),
            self.name
            ] + map(str, self.pages))


if __name__ == '__main__':
    if len(sys.argv) >= 4:
        name, fin, fout = sys.argv[1:4]
        if len(sys.argv) >= 5:
            key = open(sys.argv[4]).read().ljust(32, '\x00')[:32]
        else:
            key = PROVISION_KEY
        a = App(name)
        a.add_segment(0x100000, 5, open(fin).read(), key=key)
        a.add_segment(0x200000, 3, '\x00' * 0x10)
        open(fout, 'w').write(str(a))
