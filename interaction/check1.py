#!/usr/bin/env python2

from pwn import *
import sys

def main():

    host = sys.argv[1]
    port = int(sys.argv[2])

    conn = remote(host, port)

    result = conn.recvuntil('awesome chall\n')
    result = conn.recvuntil('who are you?\n')

    to_send = "adam"

    conn.sendline(to_send)

    result = conn.recvuntil('you said adam\n')

    sys.exit(0)


if __name__ == '__main__':
    main()
    

