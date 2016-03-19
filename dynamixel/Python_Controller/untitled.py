import serial
import math
import time
import re

def move(yaw,yawspeed,pitch,pitchspeed, camera1, camera2):
	s.write('nn,%04i,%s,%04i,%s,%s,%s,%s,%s,%s,%s,qq'%(yaw,yawspeed,pitch,pitchspeed, camera1[0], camera1[1], camera1[2], camera2[0], camera2[1], camera2[2]))

s = serial.Serial('/dev/tty.usbserial-AL01CAMG', baudrate = 57600, timeout = 3)

bob='00300'

#s.write('$1,00000,%snn'%bob)

time.sleep(1)

camera1 = []
camera2 = []

camera1.append(0)
camera1.append(0)
camera1.append(1)

a = 1
b = 1

time.sleep(1)


move(a,'0020',b,'0050',camera1, camera1)
c = s.read(300)
print c