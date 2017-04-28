import socket
from PIL import Image
import sys

mySocket =None
ip='127.0.0.1'
port=9050    

#-------
# test 1
#-------
mySocket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
print 'client A connects'
mySocket.connect(( ip,port))

# round 1
amount_received = 0
with open('mouse.txt', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE mouse.txt 917\n'
print 'client A sends \"%s\"' % message 
message += data
mySocket.sendall(message)

data = mySocket.recv(1024)
amount_received += len(data)
print 'client A should receive \"%s\"' % data

# round 2 
amount_received = 0
message = 'READ xyz.jpg 5555 2000\n'
mySocket.sendall(message)
print 'client A sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client A should receive \"%s\"' % data

# round 3
amount_received = 0
message = 'LIST\n'
mySocket.sendall(message)
print 'client A sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client A should receive \"%s\"' % data

mySocket.close()
print 'client A disconnects'


#-------
# test 2
#-------
mySocket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
print 'client B connects'
mySocket.connect(( ip,port))

# round 1
amount_received = 0
with open('mouse.txt', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE mouse.txt 917\n'
print 'client B sends \"%s\"' % message
message += data
mySocket.sendall(message)


data = mySocket.recv(1024)
amount_received += len(data)
print 'client B should receive \"%s\"' % data

# round 2 
amount_received = 0
with open('legend.txt', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE legend.txt 70672\n'
print 'client B sends \"%s\"' % message
message += data
mySocket.sendall(message)

data = mySocket.recv(1024)
amount_received += len(data)
print 'client B should receive \"%s\"' % data

# round 3
amount_received = 0
with open('chicken.txt', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE chicken.txt 31\n'
print 'client B sends \"%s\"' % message
message += data
mySocket.sendall(message)

data = mySocket.recv(1024)
amount_received += len(data)
print 'client B should receive \"%s\"' % data

# round 4
amount_received = 0
message = 'LIST\n'
mySocket.sendall(message)
print 'client B sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client B should receive \"%s\"' % data

# round 5
amount_received = 0
message = 'READ chicken.txt 4 5\n' + data
mySocket.sendall(message)
print 'client B sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client B should receive \"%s\"' % data

# round 6
amount_received = 0
message = 'READ legend.txt 50092 39\n' + data
mySocket.sendall(message)
print 'client B sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client B should receive \"%s\"' % data


mySocket.close()
print 'client B disconnects'




#-------
# test 3
#-------
amount_received = 0
mySocket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
print 'client C connects'
mySocket.connect(( ip,port))

# round 1
amount_received = 0
with open('chicken.txt', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE chicken.txt 31\n'
print 'client C sends \"%s\"' % message
message += data
mySocket.sendall(message)

data = mySocket.recv(1024)
amount_received += len(data)
print 'client C should receive \"%s\"' % data

# round 2 
amount_received = 0
with open('sonny1978.jpg', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE sonny1978.jpg 100774\n'
print 'client C sends \"%s\"' % message
message += data
mySocket.sendall(message)

data = mySocket.recv(1024)
amount_received += len(data)
print 'client C should receive \"%s\"' % data

# round 3
amount_received = 0
message = 'LIST\n'
mySocket.sendall(message)
print 'client C sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client C should receive \"%s\"' % data

# round 4
amount_received = 0
message = 'READ sonny1978.jpg 920 11\n' + data
mySocket.sendall(message)
print 'client C sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client C should receive \"%s\"' % data

# round 5
amount_received = 0
message = 'READ sonny1978.jpg 95898 3\n' + data
mySocket.sendall(message)
print 'client C sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client C should receive \"%s\"' % data

mySocket.close()
print 'client C disconnects'



#-------
# test 4
#-------

mySocket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
print 'client D connects'
mySocket.connect(( ip,port))

# round 1
amount_received = 0
with open('mouse.txt', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE mouse.txt 917\n'
print 'client D sends \"%s\"' % message
message += data;
mySocket.sendall(message)

data = mySocket.recv(1024)
amount_received += len(data)
print 'client D should receive \"%s\"' % data

# round 2 
amount_received = 0
with open('chicken.txt', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE chicken.txt 31\n'
print 'client D sends \"%s\"' % message
message += data
mySocket.sendall(message)

data = mySocket.recv(1024)
amount_received += len(data)
print 'client D should receive \"%s\"' % data

# round 3
amount_received = 0
with open('ospd.txt', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE ospd.txt 614670\n'
print 'client D sends \"%s\"' % message
message += data
mySocket.sendall(message)

data = mySocket.recv(1024)
amount_received += len(data)
print 'client D should receive \"%s\"' % data

# round 4
amount_received = 0
message = 'READ ospd.txt 104575 26\n' + data
mySocket.sendall(message)
print 'client D sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client D should receive \"%s\"' % data

# round 5
amount_received = 0
message = 'LIST\n'
mySocket.sendall(message)
print 'client D sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client D should receive \"%s\"' % data

mySocket.close()
print 'client D disconnects'




#-------
# test 5
#-------
sockudp1 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_address = ('127.0.0.1', 9095)

# round 1
amount_received = 0
message = 'LIST\n'
print 'client E sends %s' % message
sent = sockudp1.sendto(message, server_address)

# Receive response
data, server = sockudp1.recvfrom(4096)
print 'client E should receive  %s' % data

# round 2
sockudp2 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

amount_received = 0
message = 'LIST\n'
print 'client E sends %s' % message
sent = sockudp2.sendto(message, server_address)

# Receive response
data, server = sockudp2.recvfrom(4096)
print 'client E should receive  %s' % data

# round 3
sockudp3 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

amount_received = 0
message = 'LIST\n'
print 'client E sends %s' % message
sent = sockudp3.sendto(message, server_address)

# Receive response
data, server = sockudp3.recvfrom(4096)
print 'client E should receive  %s' % data



#-------
# test 6
#-------
mySocket=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
print 'client F connects via TCP'
mySocket.connect(( ip,port))

# round 1
amount_received = 0
with open('mouse.txt', 'rb') as f:
    data = f.read()
f.close()
message = 'SAVE mouse.txt 917\n'
print 'client F sends \"%s\"' % message
message += data
mySocket.sendall(message)

data = mySocket.recv(1024)
amount_received += len(data)
print 'client F should receive \"%s\"' % data

# round 2
sockudp4 = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

amount_received = 0
message = 'LIST\n'
print 'client F sends %s' % message
sent = sockudp4.sendto(message, server_address)

# Receive response
data, server = sockudp4.recvfrom(4096)
print 'client F should receive  %s via UDP' % data

# round 3
amount_received = 0
message = 'READ xyz.jpg 5555 2000\n' + data
mySocket.sendall(message)
print 'client F sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client F should receive \"%s\"' % data

# round 4
amount_received = 0
message = 'LIST\n'
mySocket.sendall(message)
print 'client F sends \"%s\"' % message

data = mySocket.recv(1024)
amount_received += len(data)
print 'client F should receive \"%s\"' % data

mySocket.close()
print 'client F disconnects via TCP'

del mySocket 
