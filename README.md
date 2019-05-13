# DEFCON 2019 Quals LCARS000

[![Build Status](https://travis-ci.com/o-o-overflow/dc2019q-LCARS000.svg?token=UsYquYpUiJ6uJsajRAzb&branch=master)](https://travis-ci.com/o-o-overflow/dc2019q-LCARS000)

This challenge contains 3 flags.

`LCARS` is a micro kernel that could load and run multiple apps. 

## Versions

LCARS000: v0.2
LCARS022: v0.4
LCARS333: v0.5

## Services
 
Services are loaded from the filesystem during booting process. They are
just static executable loaded at constant address. They can not be
modified.

### init.sys

Launching other services and interacting with user.
The first (only) service executed from kernel.

### echo.sys

### loader.sys

Parsing and loading packed binaries.

### crypto.sys

Handling all cryptography operations.

Supported opreations:

0. CRYPTO\_HASH\_MD5
1. CRYPTO\_HASH\_SHA
2. CRYPTO\_HASH\_SHA256
3. CRYPTO\_ENCRYPT\_AES
4. CRYPTO\_DECRYPT\_AES
5. CRYPTO\_DECRYPT\_RSA

Supported keys:

0. CRYPTO\_KEY\_ROOT # from './root.key'
1. CRYPTO\_KEY\_PROVISION # hardcoded 'O' * 32
2. CRYPTO\_KEY\_USER # set by user
3. CRYPTO\_KEY\_SESSION # from '/dev/urandom'

## Applets

Applets are loaded by `loader.sys`, they are packed into a special file
format.

They could be signed by `system` or `platform` keys or unsigned.

They could be encrypted by `root`, `provision`, `user` or `session` keys.

## Security Contexts

Tasks are running in different security contexts:

0. CTX\_KERNEL # unconfined
1. CTX\_SYSTEM\_APP # signed by `system` key AND encrypted by `root` key
2. CTX\_PLATFORM\_APP # signed by `platform` key AND encrypted by
   `provision` key
3. CTX\_UNTRUSTED\_APP # unsigned OR encrypted by `user` key

Tasks can only run into lower privileges.

## Kernel services

0. ECHO
1. CHECKIN
2. LOOKUP
3. WAIT
4. POST
5. OPEN
6. EXEC
7. RUNAS

## Bugs

### Level1: Information Leak in shared memory

Tasks talk to the kernel through unix sockets, parameters are passed
through shared memory. A task can access shared memory of any process, but
only has write permission to its own shared memory.

Since the `loader.sys` has to communicate with `crypto.sys` in order to
verify and decrypt apps, the encrypted app and its decryption parameters
are present in the shmem of `loader.sys`, the decrypted app is present in
the shmem of `crypto.sys`.

The hacker could write some shellcode to dump shared memory and find the
`flag1` in `flag1.papp`.

### Level2: Logic issue in signature verificaion

Each executable pages are ENCRYPTED *THEN* SIGNED. In the verification
process the loader does check if encryption mode and parameters are bound
to the signed page.

The hacker should have some signed pages from the previous stage. He could
modify the IV used in AES-CBC decryption to change the first 16 bytes of
signed page. By mapping controlled data page after current stack, he can
use ROP to open/read/write `flag2.txt`.

In this level, hacker gains code execution as PLATFORM\_APP.

`flag1.papp` is required for this level. Hacker should have solved Level1
before Level2.

### Level3: OOB read uninitialized memory

Loader records all loaded memory pages. It will unmap all pages on error.
Uninitialized .BSS data will be accessed due to an off-by-one bug. This is
not a very serious problem unless the hacker maps the page just after .BSS
section.

Now the hacker gains the ability to unmap arbitrary pages. Notice that it
does not give direct code execution primitive because after unmapping the
hacker has no way to do mapping. Unmapping .text/.stack/.data will cause
segfault immediately.

The intended solution is to unmap the `SHARED` memory and replace it with
`PRIVATE` memory. This will not affect control flow of current program, but
breaks the IPC mechanism. By unmapping `local` shared memory, remote process
will not see any update of the message content. By unmapping `remote`
shared memory, local process will see arbitrary message response. Now the
crypto verfication is totally comproised and hacker gains code execution as
SYSTEM\_APP.

Untrusted user can not talk to the loader directly. The hacker
should have at least PLATFORM\_APP privilege to talk to the loader to do
unmapping and remapping. Hacker should have solved Level2 before Level3.

### Level3+: Race condition in message forwarding

The kernel never gurantees that the consitency in message forwarding. There
are race conditions if the message reciever does not match the intended
one. This is partially mitigated by enforcing message reciever is alive and
cleaning up message queue of any dead task.

However, the message will be forwarded if the task dies and its pid is
quickly reused. The sender has no way to tell if the reciever is still
valid.

One possible exploit is to send request to the crypto without recieving the
response while keeping crypto engine busy. A new `loader` with same pid
will see the crypto engine in a broken state.

The exploit leverages bugs in message processing so PLATFORM\_APP privilege is
required.

### Backdoor1: fd leak

In v0.2, the kernel process close fds upto 1024, but the default limit of
files is more than 1024.  Untrusted apps can spray the fd by creating a lot 
of files. Then new tasks are able to hijack unix sockets between kernel and
other task with higher privilege.

Fixed in v0.3

### Backdoor2: uninitalized top

`top` is used for allocating from shared memory, is located at the
beginning of data segment. It is always initialized for services, but not
for apps, since the data segments are not signed and completely controlled
by user.

The only signed app available to users is `flag1.papp`. So this is not a
severe bug and it is probably not exploitable.

Fixed in v0.4

### Backdoor3: arbitrary mmap

The loader is supposed to be able to mmap to arbitrary address only if the
address is not in use. I (and a few other hackers) just assumed mmap to
exsiting pages with `MMAP_FIXED` will fail. This is *WRONG*.

A common exploit is to use mmap to replace current stack. This gives a few
teams much easier solution to SYSTEM\_APP privilege. However, it only
affects versions used in Level1 and Level2, and its difficulty is between
the intended solutions of these levels, so it's not so bad.

Fixed in v0.5
