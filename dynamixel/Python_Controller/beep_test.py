import serial
import math
import time
import re

s = serial.Serial('/dev/tty.usbserial-AL008IGL', baudrate = 9600)

while True:
	s.write('hicken')
	print 'hi'
	time.sleep(1)
