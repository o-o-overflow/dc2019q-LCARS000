#!/usr/bin/env python3
import os
import sys

print('awesome chall', flush=True)
print('who are you?', flush=True)

result = sys.stdin.readline().rstrip()
str = eval('"{}"'.format(result))
print("you said {}".format(str), flush=True)
