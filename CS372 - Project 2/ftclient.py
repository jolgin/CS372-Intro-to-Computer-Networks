#!/bin/python

"""
	Name: John Olgin
	Program: Project 2 - FTP client
	Course: CS 372
	Last Modified: 6/2/19
	Description:  This program will act as a simple FTP client. It will send commands to the server either
	 	for a list of files in the server's current directory or to receive a text file transferred from the 
	 	server through a data connection.
"""

import sys
import socket
import os
import time

"""
- Function Name: Connect to Server
- Description: This will create and connect the control socket from the client program to server
- Parameters: none
- Returns: integer representing control socket file descriptor
- Pre-conditions: Valid port number provided in command line arguments
- Post-conditions: An established connection and control socket will be created
- Credit: https://www.geeksforgeeks.org/socket-programming-python/
	https://pythonprogramming.net/client-server-python-sockets/?completed=/python-binding-listening-sockets/
"""

def connect_to_server():
	HOST = "flip1.engr.oregonstate.edu"
	PORT = int(sys.argv[2]);

	# create socket and connect to server to create client connection
	controlSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM);
	controlSocket.connect((HOST, PORT));
	print "Established control socket"

	return controlSocket;




"""
- Function Name: Create Data Socket
- Description: This function will set of a socket for a data connection and wait for the server to 
	attempt to connect
- Parameters: none
- Returns: valid integer for the socket file descriptor
- Pre-conditions: a valid command and port number provided in command line arguments
- Post-connections: Established data socket for the server to make connection with to send data
- Credit: https://www.geeksforgeeks.org/socket-programming-python/
	https://pythonprogramming.net/client-server-python-sockets/?completed=/python-binding-listening-sockets/
"""

def create_data_socket():
	PORT = 0

	if sys.argv[3] == "-l":
		PORT = int(sys.argv[4])
	elif sys.argv[3] == "-g":
		PORT = int(sys.argv[5])


	# create a new socket for a data connection
	try:
		dataSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	except socket.error, e:
		print "Error creating socket: %s" % e

	print ('Data socket created')


	# bind the socket
	try:
		dataSocket.bind(('', PORT))

	except socket.error as msg:
		print('Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1])

	print ('Datasocket: Socket bind complete')

	dataSocket.listen(10)
	print('Waiting for a connection...')

	# wait for incoming data connections
	serverSocket, addr = dataSocket.accept()
	print "Got connection from", addr


	return serverSocket




"""
- Function Name: Get File List
- Description: This function will retrieve and print out the list of files sent by the server
- Parameters: none
- Returns: none
- Pre-conditions: none
- Post-conditions: A list of files in the server's working directory will be printed for the user to see
"""
def get_file_list():
	dataSocket = create_data_socket()

	fileString = ""

	fileName = dataSocket.recv(50)

	print ""
	print ""
	print "File List:"

	# check for termination sequence
	if fileName != "COMPLETE":
		fileString = fileString + fileName
	
	# read in file names while COMPLETE isn't in the buffer
	while "COMPLETE" not in fileName:
		fileName = dataSocket.recv(50)

		if fileName != "COMPLETE":
			fileString = fileString + fileName

	# print the list of files for client to see
	newFileString = fileString.replace("COMPLETE", "")
	print newFileString

	print ""
	print ""
	print "Transmission Complete"

	dataSocket.close()




"""
- Function Name: Get File
- Description: This will retrieve a text file from the server's directory and write the file into the 
	client directory
- Parameters: file name and a control socket file descriptor
- Returns: none
- Pre-conditions: a valid file name provided on command line and an established control socket file descriptor
	created
- Post-conditions: A text file will be retrieved and written into the client's directory for the user to read
"""


def get_file(fileName, controlSocket):
	fileMessage = controlSocket.recv(1024)

	printMessage = "Server responded: "

	# check if file exists in client directory already
	exists = os.path.isfile(fileName)
	answer = ""

	# dataSocket = create_data_socket()


	if exists:
		answer = raw_input('File exists already in client directory. Do you want to overwrite? Enter y or n: ')

		# retrieve file if user chooses to overwrite it
		if answer == "y":
			# retrieve if the server found file
			if fileMessage == "Found file":
				dataSocket = create_data_socket()

				printMessage = printMessage + fileMessage
				print printMessage

				# open new file to write to
				file = open(fileName, "w")
				while True:

					writeBuffer = dataSocket.recv(500)

					# check for termination sequence in the buffer
					if "COMPLETE" in writeBuffer:
						newWriteBuffer = writeBuffer.replace("COMPLETE", "")
						file.write(newWriteBuffer)
						break

					else:
						file.write(writeBuffer)

				print "Transmission complete"
				dataSocket.close()

			# close if file isn't found by server
			else: 
				printMessage = printMessage + fileMessage
				print printMessage

		# close if the user decides not to overwrite
		else:
			dataSocket = create_data_socket()
			dataSocket.recv(11000000)
			time.sleep(10)
			print "Client decided not to overwrite existing file"

			dataSocket.close()

	# retrieve if the file doesn't exist (same as retrieve code above)
	else:
		if fileMessage == "Found file":
			dataSocket = create_data_socket()


			printMessage = printMessage + fileMessage
			print printMessage

			file = open(fileName, "w")
			while True:

				writeBuffer = dataSocket.recv(500)

				if "COMPLETE" in writeBuffer:
					newWriteBuffer = writeBuffer.replace("COMPLETE", "")
					file.write(newWriteBuffer)
					break

				else:
					file.write(writeBuffer)

			print "Transmission complete"
			dataSocket.close()

		else: 
			printMessage = printMessage + fileMessage
			print printMessage




"""
- Function Name: Make Request
- Description: This will control the flow of the program. It will send the appropriate commands can call 
	the appropriate functions depending on server responses and the command line arguments
- Parameters: integer for the control socket file descriptor
- Returns: none
- Pre-conditions: an established control socket file descriptor is created
- Post-conditions: The program will function appropriately depending on command line and server actions
"""

def make_request(controlSocket):
	PORT = ""

	if sys.argv[3] == "-l":
		PORT = sys.argv[4]
	elif sys.argv[3] == "-g":
		PORT = sys.argv[5]
		fileName = sys.argv[4]

	# get command from command line argument
	command = sys.argv[3]


	# send command to server
	controlSocket.send(command)
	
	# get response from server
	commandMessage = controlSocket.recv(1024)


	# if command is valid, prepare to call appropriate function
	if commandMessage == "Valid command":
		print "Server: Valid command"

		# retrieve list of files 
		if command == "-l":
			controlSocket.send(PORT)
			get_file_list()

		# retrieve and download text files
		elif command == "-g":
			controlSocket.send(PORT)
			controlSocket.send(fileName)
			get_file(fileName, controlSocket)

	# close socket and program due to invalid command
	elif commandMessage == "Invalid command":
		print "Server: Invalid command"
		controlSocket.close()
	else:
		print "Nothing sent back from server"


	controlSocket.close()
	print "Control connection closed"




if __name__ == "__main__":
	if len(sys.argv) < 5 or len(sys.argv) > 6:
		print "Invalid number of arguments. There are two options for using this program."
		print "1: python ftclient.py <SERVER_HOST> <SERVER_PORT> <COMMAND> <FILENAME> <DATA_PORT>"
		print "2: python ftclient.py <SERVER_HOST> <SERVER_PORT> <COMMAND> <DATA_PORT>"
		exit(1)

	# validate servername
	if sys.argv[1] != "flip1.engr.oregonstate.edu" and sys.argv[1] != "flip2.engr.oregonstate.edu" and sys.argv[1] != "flip3.engr.oregonstate.edu":
		print "Invalid server hostname"
		exit(1)

	# validate server port number
	port = int(sys.argv[2])
	if port <= 1024 or port > 65535:
		print "Invalid port number. Out of range"
		exit(1)

	# validate data port numbers
	if sys.argv[3] == "-l":
		port1 = int(sys.argv[4])
		if port1 <= 1024 or port1 > 65535:
			print "Invalid port number data connection. Out of range"
			exit(1)
	elif sys.argv[3] == "-g":
		port1 = int(sys.argv[5])
		if port1 <= 1024 or port1 > 65535:
			print "Invalid port number data connection. Out of range"
			exit(1)


	#connect to server and prepare to handle request
	serverSocket = connect_to_server()
	make_request(serverSocket)
	print "Client program ending now. Goodbye!"
