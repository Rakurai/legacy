import zmq
from zmq.eventloop import ioloop, zmqstream
#ioloop.install()

context = zmq.Context()
listen_socket = context.socket(zmq.STREAM)


def handle_input(msg_frames):
	# could replace msg_frames[0] with a more complex identifier than the socket num
	print("got an input: %s %s" % (msg_frames[0], msg_frames[1]))
	server_input_socket.send_multipart(msg_frames)

def handle_output(msg_frames):
	print("got an output: %s %s" % (msg_frames[0], msg_frames[1]))
	listen_socket.send_multipart(msg_frames)



if __name__ == '__main__':
	listen_port = 4000
	server_output_port = 5557 # server publishing client output
	server_input_port = 5558 # server listening for client input

	# handle IO to/from the clients connecting through (non-ZMQ) telnet clients
	listen_socket.bind("tcp://*:%d" % (listen_port))

	# subscribe to client output from the server on this socket
	server_output_socket = context.socket(zmq.SUB)
	server_output_socket.connect("tcp://localhost:%d" % (server_output_port))

	# publish client input to the server on this socket
	server_input_socket = context.socket(zmq.PUB)
	server_input_socket.bind("tcp://*:%d" % (server_input_port))

	input_stream = zmqstream.ZMQStream(listen_socket)
	input_stream.on_recv(handle_input)

	output_stream = zmqstream.ZMQStream(server_output_socket)
	output_stream.on_recv(handle_output)

	ioloop.IOLoop.instance().start()

	# cleanup

