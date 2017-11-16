#!/usr/bin/env python2
from __future__ import print_function

import os
import re
import time
import serial
import sys
import select

print ('Select the serial port')
devs = [f for f in os.listdir('/dev/') if re.match(r'tty\.|ttyU', f)]
i = 1
for dev in devs:
    print ("% d for /dev/" % i + dev)
    i += 1

input = input()
port = "/dev/%s" % devs[int(input)-1]

print (port)
# configure the serial connections (the parameters differs on the device you are connecting to)
ser = serial.Serial(
    port=port,
    baudrate=115200,
    parity=serial.PARITY_ODD,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.SEVENBITS
)

def readed(line):
    line = line.strip()
    if line == "exit":
        sys.exit(1)
    print('sending line:', line)
    ser.write(line)

if ser.isOpen():
    ser.close()
ser.open()

print ('Enter your commands below or insert "exit" to leave the application.')

while 1 :
    out = ''
    while ser.inWaiting() > 0:
        out += ser.read(1)

    if out != '':
        print (out, end = '')
        sys.stdout.flush()

    while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
        line = sys.stdin.readline()
        if line:
            readed(line)
        else: # an empty line means stdin has been closed
            print('eof')
            exit(0)
