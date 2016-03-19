import serial
import pynmea2
import re

s = serial.Serial('/dev/tty.usbserial-AL008IGL', baudrate = 9600)

#s.append()

while True:
	temp = s.readline()
	tempType = re.split(',', temp)[0]
	if tempType == '$GPRMC':
		tempParsed = pynmea2.parse(temp)
		print tempParsed	
