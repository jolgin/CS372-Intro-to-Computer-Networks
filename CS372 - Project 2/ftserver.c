/*
	Name: John Olgin
	Program: Project 2 - FTP server
	Course: CS 372
	Last Modified: 6/2/19
	Description:  This program will act as a simple FTP server. It will receive commands from a client
		and either send a list of files in the current directory or transfer a text file directly to the
		directory the client is running from.
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <signal.h>


struct addrinfo * create_address(char *port_number, int isDataPort);
int create_socket(struct addrinfo *p);
void listen_socket(int sockfd);
void await_connection(int socket_file_descriptor);
char **list_of_files();
int num_of_files();
char **create_file_list();
int check_for_file(char **fileList, char *fileName);
void handle_request(int socketFD);
void send_file_list(char *port_number, int controlSocketFD);
void send_file(char*port_number, char *fileName, int controlSocketFD);
void sigint_handler(int s);
volatile int running = 1; 

char fileSendBuffer[12000000000];

int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "Usage: ./ftserver <port number> \n");
	}

	// validate port number
	int port = atoi(argv[1]);
	if((port <= 1024) || (port > 65535)){
		fprintf(stderr, "Invalid port number\n");
		exit(1);
	}

	char  *port_number = argv[1];

	//prepare server socket to wait for connection
	struct addrinfo *serverInfo = create_address(port_number, 0);
	int socket_file_descriptor = create_socket(serverInfo);
	listen_socket(socket_file_descriptor);

	//start up server to accept connections
	await_connection(socket_file_descriptor);
	fprintf(stderr, "Exiting program, goodbye!\n");
	return 0;
}



/**************************************
- Function Name: Create Address
- Description: This will fill and prepare a addrinfo object to be filled to be used as a socket
- Parameters: Port number and integer signifying if the socket is for a data connection to a client
- Returns: addrinfo subject
- Pre-conditions: A valid port name is provided via the command line 
- Post-conditions: A valid addrinfo ready to be used in a socket connection
- Credit: Beej's guide
****************************************/

struct addrinfo * create_address(char *port_number, int isDataPort) {
	int rv;
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	//set addrinfo variables 
	if(isDataPort == 0){
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;


		if((rv = getaddrinfo(NULL, port_number, &hints, &serverInfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}
	}

	// This section will run only of the server is needing a data socket connection
	else if(isDataPort == 1) {
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;

		if((rv = getaddrinfo("128.193.54.182", port_number, &hints, &serverInfo)) != 0) {
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
			return 1;
		}
	}
	return serverInfo;
}







/**************************************
- Function Name: Create Socket
- Description: This function will develop a socket file descriptor to be used for incoming connections to client
- Parameters: A pointer to an addrinfo object
- Returns: An integer for the socket file descriptor
- Pre-conditions: A valid addrinfo struct was created by the create_address function
- Post-conditions: A valid socket file descriptor will be returned to be used for client connections
- Credit: Beej's guide
****************************************/
int create_socket(struct addrinfo *p) {
	int socket_file_descriptor;

	//create file descriptor for control connection
	if((socket_file_descriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		perror("server: socket");
		 exit(1);
	} else {
		fprintf(stderr, "SocketFD is good\n");
	}


	//bind to host
	if(bind(socket_file_descriptor, p->ai_addr, p->ai_addrlen) == -1) {
		close(socket_file_descriptor);
		perror("server: bind");
		exit(1);
	} else {
		fprintf(stderr, "Socket bind is good\n");

	}

	return socket_file_descriptor;
}




/**************************************
- Function Name: Listen Socket
- Description: This will force the server to listen and prepare to accept incoming connections
- Parameters: Socket file descriptor integer
- Returns: nothing
- Pre-conditions: A valid socket file descriptor is created by the create_socket function
- Post-conditions: Function will tell the user if the server is properly listening 
- Credit: Beej's guide
****************************************/
void listen_socket(int sockfd) {
	if(listen(sockfd, 5) == -1) {
		perror("listen");
		exit(1);
	} else {
		fprintf(stderr, "Listening...\n");
	}
}





/**************************************
- Function Name: Create Data Socket
- Description: This creates a socket file descriptor for a data connection with the client
- Parameters: A pointer to an addrinfo object
- Returns: An integer for the socket file descriptor for data connection 
- Pre-conditions: A valid addrinfo struct was created by the create_address function
- Post-conditions: A valid socket file descriptor will be returned to be used for data connections
- Credit: Beej's guide
****************************************/
int create_data_socket(struct addrinfo *p) {
	int socket_file_descriptor;

	//create socket file descriptor for data connection
	if((socket_file_descriptor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
		perror("data: socket");
		 exit(1);
	} else {
		fprintf(stderr, "Data SocketFD is good\n");
	}

	return socket_file_descriptor;
}





/**************************************
- Function Name: Connect Data Socket
- Description: This connects to the data socket created by the client for data transfer
- Parameters: data socket file descriptor, pointer to addrinfo object
- Returns: nothing
- Pre-conditions: Valid socket file desriptor and addrinfo object created
- Post-conditions: Server will either be connected or there will be an error message
- Credit: My project 1 code
****************************************/
void connect_data_socket(int dataSocketFD, struct addrinfo *clientInfo){
	int connectStatus;
	if((connectStatus = connect(dataSocketFD, clientInfo->ai_addr, clientInfo->ai_addrlen)) == -1){
		fprintf(stderr, "Data socket connection error\n");
		exit(1);
	}
}





/**************************************
- Function Name: Sigint handler
- Description: Switches a variable to get out while loop waiting for client connection
- Parameters: integer for the type of signal received
- Returns: nothing
- Pre-conditions: SIGINT must be entered as keyboard input by the user
- Post-conditions: Program ends
- Credit: Beej's guide
****************************************/
void sigint_handler(int s){
	running = 0;
}





/**************************************
- Function Name: Await Connection
- Description: This is the "start up" function. It starts the server to wait for client connections and handle
	incoming requests
- Parameters: integer as a socket file descriptor
- Returns: nothing
- Pre-conditions: A valid socket file descriptor created 
- Post-conditions: The server is prepared and waiting for a connection from a client
- Credit: Beej's guide for both the while loop and sig int handler
****************************************/
void await_connection(int socket_file_descriptor) {
	struct sockaddr_storage client_address;
	socklen_t client_address_size;


	// create a sigaction object to prepare to handle a SIGINT keyboard input
	struct sigaction sa;
	sa.sa_handler = sigint_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);

	// Loop and wait for connection from client
	while(running == 1){
		fprintf(stderr, "Waiting for a connection...\n");
		client_address_size = sizeof(client_address);

		// start accepting connections
		int new_fd = accept(socket_file_descriptor, (struct sockaddr *)&client_address, &client_address_size);

		// keep looping or handle request from incoming connection
		if(new_fd == -1) {
			continue;
		} else {
			fprintf(stderr, "Established control socket\n");
			handle_request(new_fd);
			close(new_fd);
		}
	}

	close(socket_file_descriptor);
}





/**************************************
- Function Name: List of Files
- Description: This creates a list of files from the current directory as an array of strings
- Parameters: none
- Returns: A pointer to an array of strings
- Pre-conditions:  none
- Post-conditions: A list with all the available files in the current directory
- Credit: https://www.sanfoundry.com/c-program-list-files-directory/
****************************************/
char **list_of_files(){

	// prepare variables to read from current directory
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	int index = 0;

	// create an array of strings
	char **fileArray = create_file_list();


	// read names into the created array of strings
	if(d){
		while((dir = readdir(d)) != NULL) {
			if(dir->d_type == DT_REG ){
				//printf("%s\n", dir->d_name);
				strcpy(fileArray[index], dir->d_name);
				index++;
			}	
		}

		closedir(d);
	}

	return fileArray;
}





/**************************************
- Function Name: Num of Files
- Description: This will just calculate the total number of files within the directory
- Parameters: none
- Returns: an integer representing the number of files
- Pre-conditions: none
- Post-conditions: A count of the files will be provided
- Credit: https://www.sanfoundry.com/c-program-list-files-directory/
****************************************/
int num_of_files(){

	// prepare variables to count the number of files in the current directory
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	int numberOfFiles = 0;

	// count the number of files in the current directory
	if(d){
		while((dir = readdir(d)) != NULL) {
			if(dir->d_type == DT_REG ){
				numberOfFiles++;
			}
		}

		closedir(d);
	}

	return numberOfFiles;
}





/**************************************
- Function Name: Create File List
- Description: This will allocate space to hold the string of filenames
- Parameters: none
- Returns: An allocated empty array of strings
- Pre-conditions: none
- Post-conditions: Valid space will be created and prepared to hold the list of files
- Credit: https://stackoverflow.com/questions/27883242/how-to-dynamically-allocate-an-array-of-strings-in-c
****************************************/
char **create_file_list(){
	int listSize = num_of_files();

	// allocate a string of arrays large enough to hold the number of files
	char **fileList = malloc(listSize*sizeof(char *));
	int i;
	for(i = 0; i < listSize; i++){
		fileList[i] = malloc(50*sizeof(char));
		memset(fileList[i], 0, sizeof(fileList[i]));
	}

	return fileList;
}






/**************************************
- Function Name: Free File List
- Description: This will free the memory holding the list of file names
- Parameters: An array of strings
- Returns: none
- Pre-conditions: An existing list must have been created
- Post-conditions: Memory will be freed to avoid memory leaks
****************************************/
void free_file_list(char **fileList){
	int listSize = num_of_files();
	int i;

	// free all the allocated memory for the list
	for(i = 0; i < listSize; i++){
		free(fileList[i]);
	}
	free(fileList);
}




/**************************************
- Function Name: Check For File
- Description: This will determine if the requested file by the client exists in the server's directory
- Parameters: list of files, specific file name
- Returns: an integer representing whether or not the file exists
- Pre-conditions: A valid file list is created and file name provided by client
- Post-conditions: Will tell the users whether or not the file exists
****************************************/
int check_for_file(char **fileList, char *fileName){
	int listSize = num_of_files();
	int file = 0;
	int i;

	//iterate through the file list to see if the requested file exists
	for(i = 0; i < listSize; i++){
		// if file is found
		if(strcmp(fileList[i], fileName) == 0){
			file = 1;
			break;
		} 
	}

	return file;
}





/**************************************
- Function Name: Handle Request
- Description: This will receive the commands and control the program flow dependning on those commands by calling
	other functions 
- Parameters: a socket file descriptor
- Returns: none
- Pre-conditions: a valid socket file descriptor is created
- Post-conditions: Program will flow appropriately depending on commands, file names, etc.
****************************************/
void handle_request(int socketFD){
	char command[20];
	char port_number[20];
	memset(command, 0, sizeof(command));
	memset(port_number, 0, sizeof(port_number));

	// receive and print the command provided by the client
	recv(socketFD, command, sizeof(command)-1, 0);
	printf("Client sent the command: %s \n", command);


	// send the client a message notifying the command is invalid and leave handle request function 
	if((strcmp(command, "-l") != 0) && (strcmp(command, "-g") != 0)){
		char *badCommand = "Invalid command";
		fprintf(stderr, "Client sent an invalid command and will close the connection\n");
		send(socketFD, badCommand, strlen(badCommand), 0);

		return;
	}

	// run appropriate commands for the list (-l) command
	if(strcmp(command, "-l") == 0){
		// confirm valid command to user
		char *confirm = "Valid command";
		send(socketFD, confirm, strlen(confirm), 0);

		//receive port number for data connection
		recv(socketFD, port_number, sizeof(port_number)-1, 0);

		// tell user on server of intention to send file list
		fprintf(stderr, "The client has requested a list of files in the current directory...\n");
		fprintf(stderr, "Sending the list to port %s \n", port_number);
		send_file_list(port_number, socketFD);
	}

	// run appropriate command for get (-g) command to send file
	else if(strcmp(command, "-g") == 0) {
		char fileName[50];
		memset(fileName, 0, sizeof(fileName));

		//confirm valid command to user
		char *confirm = "Valid command";
		send(socketFD, confirm, strlen(confirm), 0);

		//receive port number and file name from client
		recv(socketFD, port_number, sizeof(port_number)-1, 0);
		recv(socketFD, fileName, sizeof(fileName)-1, 0);

		// print out data connection port and requested file 
		fprintf(stderr, "Client is requesting the file %s to port %s\n", fileName, port_number);


		//create file list and check for specific file requested by client
		char **fileList = list_of_files();
		int isFileThere = check_for_file(fileList, fileName);


		// Let client know the file is found and send it
		if(isFileThere == 1){
			fprintf(stderr, "The file requested exists. Transfer initiated...\n");
			char *found = "Found file";
			send(socketFD, found, strlen(found), 0);
			send_file(port_number, fileName, socketFD);
		}

		// let client know the file wasn't found 
		else if(isFileThere == 0){
			fprintf(stderr, "Client requested a nonexistent file from the current directory\n");
			char *notFound = "File not found";
			send(socketFD, notFound, strlen(notFound), 0);
		}

		free_file_list(fileList);
	}

}





/**************************************
- Function Name: Send File List
- Description: This will actually transfer the names of the files in the directory to the client
- Parameters: port number, socket file descriptor for control connection
- Returns: none
- Pre-conditions: valid port number and control socket file descriptor created earlier in the program
- Post-conditions: List of files will be transferred via the data connection to the client
****************************************/
void send_file_list(char *port_number, int controlSocketFD) {

	//set up for the data socket and connect
	sleep(2);
	struct addrinfo *dataInfo = create_address(port_number, 1);
	int data_socket_FD = create_data_socket(dataInfo);
	connect_data_socket(data_socket_FD, dataInfo);

	//create the file list and get total number of files
	char **fileList = list_of_files();
	int numberOfFiles = num_of_files();


	//loop to send every file name
	int i;
	for(i = 0; i < numberOfFiles; i++) {
		send(data_socket_FD, fileList[i], strlen(fileList[i]), 0);
		send(data_socket_FD, "   ", 3, 0);
	}

	//let client know there are no more file names left
	char *complete = "COMPLETE";
	send(data_socket_FD, complete, strlen(complete), 0);

	fprintf(stderr, "Directory transfer complete\n");

	//cleanup
	close(data_socket_FD);
	fprintf(stderr, "Data socket closed\n");
	freeaddrinfo(dataInfo);
	free_file_list(fileList);
}





/**************************************
- Function Name: Send File
- Description: This sends the actual contents of the requested text file to the client
- Parameters: string for port number, string for file name, integer for control socket file descriptor
- Returns: none
- Pre-conditions: A valid port number, file name, and socket file descriptor are provided/created earlier in
	the program
- Post-conditions: The text file will be sent via the data connection to the client's working directory
- Credit: Strongly influenced by https://stackoverflow.com/questions/11229230/sending-and-receiving-a-file-server-client-in-c-using-socket-on-unix
****************************************/
void send_file(char *port_number, char *fileName, int controlSocketFD){
	//set up for the data socket and connect
	sleep(5);
	struct addrinfo *dataInfo = create_address(port_number, 1);
	int data_socket_FD = create_data_socket(dataInfo);
	connect_data_socket(data_socket_FD, dataInfo);

	memset(fileSendBuffer, 0, sizeof(fileSendBuffer));

	// check if open file was successful
    int file = open(fileName, O_RDONLY);
    if(file < 0){
        printf("Unable to open file\n");
        exit(1);
    }
    
    // record the number of bytes read from the file
    int readBytes = read(file, fileSendBuffer, sizeof(fileSendBuffer) - 1);
    

    // send bytes without having to count since total bytes is less than 500
    if(readBytes <= 500){
    	send(data_socket_FD, fileSendBuffer, readBytes , 0);
        send(data_socket_FD, "COMPLETE", 8, 0);
    }

    // send file while breaking up in increments of 500 from the buffer holding the file contents
    else {
    	int sentBytes = 0;
    	int charCounter;
        int fileBufferCounter = 0;
        int remainingBytes = readBytes;
        char sendBuffer[500];
        

        //while there are still bytes to be sent file the send buffer from file buffer
        while(remainingBytes > 0){
            for(charCounter = 0; charCounter < 500; charCounter++){
                sendBuffer[charCounter] = fileSendBuffer[fileBufferCounter];
                fileBufferCounter++;
            }
            
            // actually send the data
            int written = send(data_socket_FD, sendBuffer, 500, 0);
        
        	// keep track of sent bytes and remaining bytes to be sent
            sentBytes += written;
            int remainingBytes = readBytes - sentBytes;
            
            //send all remaining bytes and exit while loop
            if(remainingBytes < 500){
                memset(sendBuffer, 0, sizeof(sendBuffer));

                // fill the send buffer again from remaining bytes in file buffer
                for(charCounter = 0; charCounter < remainingBytes; charCounter++){
                    sendBuffer[charCounter] = fileSendBuffer[fileBufferCounter];
                    fileBufferCounter++;
                }
                send(data_socket_FD, sendBuffer, remainingBytes, 0);
                
                //send termination sequence of characters
                send(data_socket_FD, "COMPLETE", 8, 0);
                remainingBytes = 0;
                break;
            }
        }
    }

 
	fprintf(stderr, "File transfer complete\n");
	fprintf(stderr, "Data socket closed\n");
	close(data_socket_FD);
	freeaddrinfo(dataInfo);
}













