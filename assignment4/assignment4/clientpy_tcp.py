import socket
import sys

def get_constants(prefix):
    """Create a dictionary mapping socket module constants to their names."""
    return dict( (getattr(socket, n), n)
                 for n in dir(socket)
                 if n.startswith(prefix)
                 )

families = get_constants('AF_')
types = get_constants('SOCK_')
protocols = get_constants('IPPROTO_')

# Create a TCP/IP socket
sock = socket.create_connection(('localhost', 9050))

'''
print >>sys.stderr, 'Family  :', families[sock.family]
print >>sys.stderr, 'Type    :', types[sock.type]
print >>sys.stderr, 'Protocol:', protocols[sock.proto]
print >>sys.stderr
'''
try:
    # Send data
    
    message = 'SAVE a.txt 20\njuhsgjuhsgjuhsgjuhsg'
    print >>sys.stderr, 'sending %s' % message
    sock.sendall(message)

    amount_received = 0
    amount_expected = len(message)
    
    
    data = sock.recv(1024)
    amount_received += len(data)
    print >>sys.stderr, 'received %s' % data
    

    message = 'READ a.txt 3 10\n'
    print >>sys.stderr, 'sending %s' % message
    sock.sendall(message)

    amount_received = 0
    amount_expected = len(message)
    
    data = sock.recv(1024)
    amount_received += len(data)
    print >>sys.stderr, 'received %s' % data
    

    message = 'LIST\n'
    print >>sys.stderr, 'sending %s' % message
    sock.sendall(message)

    amount_received = 0
    amount_expected = len(message)
    
    data = sock.recv(1024)
    amount_received += len(data)
    print >>sys.stderr, 'received %s' % data


finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
