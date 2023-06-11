# sudo python3 measure.py
from bluepy.btle import Scanner
import fcntl

my_dev={'b8:27:eb:4f:5e:43':'Beacon1','b8:27:eb:a1:e3:c6':'Beacon2','b8:27:eb:5c:71:2d':'Beacon3'}

f_log = open('log', 'a')

scanner = Scanner()
while(True):
    devices = scanner.scan(0.3)
    out = {'Beacon1':'0', 'Beacon2':'0', 'Beacon3':'0'} 
    for device in devices:
        if device.addr in my_dev.keys():
            out[my_dev[device.addr]]=str(device.rssi)
    result = " ".join(out.values())
    f_curr = open('curr','w')
    fcntl.flock(f_curr.fileno(), fcntl.LOCK_SH)
    f_curr.write(result)
    fcntl.flock(f_curr.fileno(), fcntl.LOCK_UN)
    f_curr.close()

    f_log.write(result+'\n')
f_log.close()
