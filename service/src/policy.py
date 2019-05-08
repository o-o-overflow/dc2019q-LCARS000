import sys
from seccomp import *

f = SyscallFilter(KILL)
f.add_rule(ALLOW, "exit")
f.add_rule(ALLOW, "read")
f.add_rule(ALLOW, "write")
f.add_rule(ALLOW, "munmap")
print 'untrusted apps'
f.export_pfc(sys.stdout)
f.export_bpf(open('untrusted.bpf', 'w'))

f.add_rule(ALLOW, "close")
# f.add_rule(ALLOW, "sendmsg") # not really used
f.add_rule(ALLOW, "recvmsg")
print 'plataform apps'
f.export_pfc(sys.stdout)
f.export_bpf(open('platform.bpf', 'w'))

f.add_rule(ALLOW, 'mmap')
f.add_rule(ALLOW, 'mprotect')
f.add_rule(ALLOW, 'prctl')
f.add_rule(ALLOW, "writev") # for dprintf
print 'system apps'
f.export_pfc(sys.stdout)
f.export_bpf(open('system.bpf', 'w'))
