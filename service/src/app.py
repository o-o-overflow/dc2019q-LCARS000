import sys, struct

class Page(object):
    def __init__(self, base, prot, raw=''):
        assert len(raw) <= 0x1000
        self.base = base
        self.raw = raw
        self.prot = prot

    def encrypt(self, key):
        pass

    def __str__(self):
        return struct.pack('<III', self.base,
                len(self.raw), self.prot) + self.raw

class App(object):
    def __init__(self, name='app'):
        self.name = name.ljust(0x20, '\x00')[:0x20]
        self.pages = []

    def add_segment(self, base, prot, raw):
        for i in xrange(0, len(raw), 0x1000):
            self.pages.append(Page(base + i, prot, raw[i:i + 0x1000]))

    def __str__(self):
        return ''.join([
            'EFIL',
            struct.pack('<I', len(self.pages)),
            self.name
            ] + map(str, self.pages))


if __name__ == '__main__':
    # test
    a = App('hello')
    a.add_segment(0x8000, 5, open('hello.bin').read())
    open('hello.app', 'w').write(str(a))
