import zmq

listen_port = 4000
server_port = 5556

context = zmq.Context()
listen_socket = context.socket(zmq.STREAM)
listen_socket.bind("tcp://*:" + str(listen_port))

server_socket = context.socket(zmq.DEALER)
server_socket.connect("tcp://localhost:" + str(server_port))

while True:
	client_id, msg = listen_socket.recv_multipart()
	print("proxy: client %s sent '%s'", client_id, msg)

	server_socket.send(msg)
	reply = server_socket.recv()

	print("proxy: server replied '%s'", reply)
	listen_socket.send_multipart([client_id, reply])
