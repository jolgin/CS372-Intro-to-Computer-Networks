#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/*
	Name: John Olgin
	Project: Project 1
	Description: This program is a simple TCP chat client that establishes a connection 
		with the server and allows the two to exchange messages.
	Course Name: CS 372 Intro to Computer Networks
	Modified: 5/5/19
*/

//int connect_to_server(char *host_name, int port_number);
char *create_handle();
int send_message(int socket_file_descriptor, char *user_handle);
int receive_message(int socket_file_descriptor);

int main(int argc, char *argv[])
{
	//ensure the correct amount of command line arguments are provided
	if(argc != 3) {
		fprintf(stderr, "Usage: chatclient <hostname> <port number> \n");
	}

	//pull hostname and port number from command line arguments
	char hostname[100];
	int port_number = atoi(argv[2]);
	strcpy(hostname, argv[1]);

	// create user handle
	char *user_handle = create_handle();
	
	//establish connection to server and get socket file descriptor
	int socket_file_descriptor = connect_to_server(*hostname, port_number);




	//prepare for communication with the server
	//checker variables to determine if either party has quit
	int did_client_quit = 0;
	int did_server_quit = 0;

	//while no one has quit, send and receive messages with the server user
	while(did_server_quit == 0 && did_client_quit == 0)
	{
		did_client_quit = send_message(socket_file_descriptor, user_handle);
		did_server_quit = receive_message(socket_file_descriptor);
	}


	printf("Connection terminated, the program will now end.\n");
	close(socket_file_descriptor);

	return 0;
}


/*	Function: connect_to_server()
	Description: This function will generate a connection with the server. It will also
		generate a socket file descriptor.

	Pre-conditions: The hostname and port number need to be pulled from the command
		line arguments and passed into this function. 

	Post conditions: A connection to the server will be established. Thus, a socket
		file descriptor will be obtained and returned in integer form to main()

	Credit: Much of this function was taken from this link:
		https://www.geeksforgeeks.org/socket-programming-cc/
*/
int connect_to_server(char* host_name, int port_number)
{
	struct sockaddr_in serv_addr;
	int client_socket = 0;


	//check if successful socket is made
	if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Unable to create socket");
		return -1;
	}

	//clear out the serv_addr object
	memset(&serv_addr, '0', sizeof(serv_addr));


	//set family and port of serv_addr object
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_number);


	//translate IP address to binary, ensure it's valid
	if(inet_pton(AF_INET, "128.193.54.168", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    }


    //establish connection to server and obtain socket file descriptor
    if(connect(client_socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
    	fprintf(stderr, "Connection error");
    	return -1;
    }

	return client_socket;
}



/*	Function: create_handle()
	Description: This function allows a user to create a handle thats 10 or less characters.

	Pre-conditions: No conditions need to be met before this function is called.

	Post conditions: A user handle will be created and passed back to main() for use
		later in the program.
*/
char *create_handle()
{
	char handle_buffer[11];

	//Allow user to enter input for user name (handle)
	printf("Please enter a username of 10 characters or less: \n");
	scanf("%11s", handle_buffer);
  	

  	// get length of user to check validity
	int username_length = strlen(handle_buffer);


	//if username is too long, prompt user for another handle until its valid.
	while(username_length > 10)
	{
		int ch;

		//flush stdin
		while ((getchar()) != '\n'); 
		printf("The username you entered is too long. Please enter a username of 10 characters of less: \n");
		scanf("%99s", handle_buffer);

		username_length = strlen(handle_buffer);
	}


	//append ">" character to handle
	strcat(handle_buffer, ">");

	//flush stdin
	while ((getchar()) != '\n');


	return handle_buffer;
}



/*	Function: send_message()
	Description: Allows the client user to send a message to the server of 500 or less
		characters.

	Pre-conditions: A socket file descriptor and string need to be developed and passed
		into the function. Therefore, a successful connection with the server needs to be
		made. A user handle also needs to be made prior to calling the function.

	Post conditions: Neither variables will be altered. A message will be displayed that 
		the user inputs. An integer will be returned stating whether or not the client
		user quit.
*/
int send_message(int socket_file_descriptor, char *user_handle)
{
	//create array to hold the final message and copy the user handle into it
	char final_message[514];
	strcpy(final_message, user_handle);

	printf("%s ", user_handle);


	//user input for message to the server, only take 500 characters
	char message_buffer[501];
	scanf("%500[^\n]", message_buffer);
	while ((getchar()) != '\n');


	int message_length = strlen(message_buffer);


	//build full message with handle, message buffer, and newline character
	strcat(final_message, " ");
	strcat(final_message, message_buffer);
	strcat(final_message, "\n");
	

	//store however many characters were successfully sent
	int characters_sent = send(socket_file_descriptor, final_message, strlen(final_message), 0);



	//check for error sent back from send()
	if (characters_sent == -1) 
	{
		fprintf(stderr, "Error: message wasn't sent\n");
		exit(1);
	}


	//check if user entered the \quit command
	if(strcmp(message_buffer, "\\quit") == 0)
	{
		printf("you quit!\n");
		return 1;
	}

	return 0;
}



/*	Function: receive_message()
	Description: This will allow the client user to receive a string message from the
		server. It will display it for the user to see.

	Pre-conditions: A fully developed socket file descriptor needs to be built and passed
		into the function. Thus, a successful connection needs to be made with the server
		to receive the socket file descriptor.

	Post conditions: The socket file descriptor variable will not be altered at all. A
		message from the server will be displayed to the client user. An integer will be 
		returned signifying if the user has quit.
*/
int receive_message(int socket_file_descriptor) {
	char received_message[514];


	//store characters received successfully from recv()
	int characters_received = recv(socket_file_descriptor, received_message, sizeof(received_message) - 1, 0);


	//check for error sent back from recv()
	if(characters_received == -1)
	{
		fprintf(stderr, "Error: no message received");
		exit(1);
	}


	//check if user entered the \quit command
	if(strstr(received_message, "\\quit") != NULL)
	{
		return 1;
	}

	printf("%s\n", received_message);

	return 0;
}







