#!/usr/bin/env python2
from __future__ import print_function
import cmd
import os
import re
import serial
import thread
import time
import readline
import atexit
from time import sleep

BAUD = 115200


if __name__ == '__main__':

    print ('Select the serial port')
    devs = [f for f in os.listdir('/dev/') if re.match(r'tty\.|ttyU', f)]
    i = 1
    for dev in devs:
        print ("% d for /dev/" % i + dev)
        i += 1

    input = input()
    port = "/dev/%s" % devs[int(input)-1]

    ser = serial.Serial(
        port = port,
        baudrate = BAUD,
    )
    if ser.isOpen():
        ser.close()
    ser.open()
    ser.read(1)

    line = []
    while 1:
        for c in ser.read():
            line.append(c)
            if c == '\n':
                l = ''.join(line)
                print (l.strip())
                line = []
                break
