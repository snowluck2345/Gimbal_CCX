import serial
import math
import time
import re

def move(yaw,yawspeed,pitch,pitchspeed, camera1, camera2):
	s.write('nn,%04i,%s,%04i,%s,%s,%s,%s,%s,%s,%s,qq'%(yaw,yawspeed,pitch,pitchspeed, camera1[0], camera1[1], camera1[2], camera2[0], camera2[1], camera2[2]))

s = serial.Serial('/dev/tty.usbserial-AL008IGL', baudrate = 57600, timeout = 1)

bob='00300'

#s.write('$1,00000,%snn'%bob)
#rawinput, len

time.sleep(1)

camera1 = []
camera2 = []

camera1.append(0)
camera1.append(0)
camera1.append(1)

time.sleep(1)

for a in range (0,4050,340):
	for b in range(0,2200,340):
		move(b,'0030',a,'0015',camera1, camera1)
		c = s.read(1000)
		print c

		camera1[0]=1
		camera1[1]=1
		time.sleep(.5)
		move(b,'0030',a,'0015',camera1, camera1)
		c = s.read(1000)
		print c

		camera1[0]=0
		camera1[1]=0
		time.sleep(.5)
		print 'hi'
