import sys
import socket
# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

server_address = ('127.0.0.1', 9095)

try:

    # Send data
    message = 'READ a.txt 3 10\n'
    print >>sys.stderr, 'sending %s' % message
    sent = sock.sendto(message, server_address)

    # Receive response
    print >>sys.stderr, 'waiting to receive'
    data, server = sock.recvfrom(4096)
    print >>sys.stderr, 'received %s' % data

    message = 'LIST\n'
    print >>sys.stderr, 'sending %s' % message
    sent = sock.sendto(message, server_address)

    # Receive response
    print >>sys.stderr, 'waiting to receive'
    data, server = sock.recvfrom(4096)
    print >>sys.stderr, 'received %s' % data

finally:
    print >>sys.stderr, 'closing socket'
    sock.close()
