import zmq
import time
import sys

if __name__ == '__main__':

    input_port = 5558
    output_port = 5557

    context = zmq.Context()
    output_socket = context.socket(zmq.PUB)
    output_socket.bind("tcp://*:%i" % output_port)
    input_socket = context.socket(zmq.SUB)
    input_socket.connect("tcp://localhost:%i" % input_port)

    players = {}
#    poller = zmq.Poller()
#    poller.register(input_socket, zmq.POLLIN)

    while True:
#        startpoll = time.time()
#        waiting = poller.poll(1000)
#        elapsed = time.time() - startpoll

#        if waiting:

#            print(waiting)
#            if waiting.get(input_socket) == zmq.POLLIN:

        client_id, msg = input_socket.recv_multipart()
        print("server: %s sent '%s'" % (client_id, msg))
        output_socket.send_multipart([client_id, bytes("hello, %s" % client_id, 'utf-8')])
#        else:
#            print("server: no sockets ready for reading")

#        elapsed = time.time() - loop_start_t
#        time.sleep(max(0, 1 - elapsed))
