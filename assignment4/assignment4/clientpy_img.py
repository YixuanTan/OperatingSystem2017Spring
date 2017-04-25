import socket
from PIL import Image
import sys

mySocket =None
ip='127.0.0.1'
port=9050     
try :
    print 'starting Client'
    mySocket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)

    print 'trying to connet'
    mySocket.connect(( ip,port))
    print 'conneted successfuly!'
except socket.error,msg :
    print 'Failed to create socket.Error code :' +str(msg[0])+'Error message:'+msg[1]


message = 'READ test.png 0 25099\n'
print >>sys.stderr, 'sending %s' % message
mySocket.sendall(message)

amount_received = 0
amount_expected = 25099


buf = b''

while amount_received < amount_expected:
    data = mySocket.recv(amount_expected - amount_received)
    amount_received += len(data)
    buf += data
    print >>sys.stderr, 'received accumaltive %d' % amount_received

with open('return.png', 'wb') as f:
    f.write(buf)
f.close()



mySocket.close()
del mySocket 
