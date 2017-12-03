#/usr/bin/python

import sys, io

bufsize = 2**16
padding = 16
act_size = bufsize - padding

def input_check(a):
    a = str(raw_input(a))
    if len(a) <= act_size:
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
    n = int(raw_input('How many questions? '))
    while 1 < n < act_size:
        n = int(raw_input('Please enter number 1 .. ' + str(bufsize - padding)))
    while n:
        write_token(key, input_check('Question ' + str(n) + ' : '), f)
        write_token(key, input_check('Right answer ' + str(n) + ' : '), f)
        n -= 1
f.close()
