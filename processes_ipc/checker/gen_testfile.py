#/usr/bin/python

import sys, io

def input_check(a):
    a = str(raw_input(a))
    if len(a) < 2**16:
        return a.strip()
    else:
        print('Too long input! Bye!')
        sys.exit(1)

def write_token(k, s, fd):
    z = ''
    j = 0
    l = len(s)
    for i in s:
        z += chr(ord(i) ^ (ord(k[j % len(k)]) - 2 * j * (j & 1)))
        j += 1
    fd.write(bytearray(chr((l & (255 << 8)) >> 8), "utf8"))
    fd.write(bytearray(chr(l & 255), "utf8"))
    fd.write(bytearray(z, "utf8"))


with io.open(sys.argv[1], "wb") as f:
    key = input_check('Enter key: ')
    dbg = ''
    for i in key:
        dbg += hex(ord(i)) + " "
    print('Your key: ' + dbg)
    write_token(key, input_check('Enter name of test topic: '), f)
    n = int(input_check('How many questions? '))
    while n:
        write_token(key, input_check('Question: '), f)
        write_token(key, input_check('Right answer: '), f)
        n -= 1
f.close()
