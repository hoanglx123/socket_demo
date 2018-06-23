import socket
import sys
import thread

# Connection handler
def receive_handler(sock):
    while True:
        # Receive data from server
        data = sock.recv(512)

        if data:
            print "\nSERVER: ", data
        else : 
            break

    thread.exit_thread()

def main():
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect the socket to the port where the server is listening
    server_address = ("127.0.0.5", 8888)
    print >>sys.stderr, "connecting to %s port %s" % server_address
    sock.connect(server_address)

    try: 
        thread.start_new_thread(receive_handler, (sock, ))

        while True:
            # Send data to server 
            data = raw_input ( "Enter type: " )
            sock.send(data)

    finally:
        print >>sys.stderr, 'closing socket'
        sock.close()

# Run program
main()