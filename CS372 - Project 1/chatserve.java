/*
	Name: John Olgin
	Project: Project 1
	Description: This program is a simple TCP chat server that establishes a connection 
		with a client and allows the two to exchange messages.
	Course Name: CS 372 Intro to Computer Networks
	Modified: 5/5/19
*/
import java.net.*;
import java.io.*;


public class chatserve {

	// Credit: Some of this function was taken from the oracle documents provided 
	// 		in the project1.pdf. Mainly the creation of the ServerSocket object, 
	//		PrintWriter out object, and BufferedeReader in object. I also got help
	//      for the if statement that checks the command line arguments from those docs
	public static void main(String[] args) throws IOException {
		//ensure the correct amount of arguments were provided
		if (args.length != 1) {
			System.err.println("Usage: java chatServer <port number>");
			System.exit(1);
		}


		// read and save the port number given from the command line argument
		int portNumber = Integer.parseInt(args[0]);


		// Introduction message
		System.out.println("");
		System.out.println("Chat server now running on port: "  + portNumber);
		System.out.println("This is a chat server that allows client connections.");
		System.out.println("It enables two users to chat with each other from different machines");
		System.out.println("PRESS CTRL C to exit the program!");
		System.out.println("");

		//socket for client to connect to
		ServerSocket socketServer = new ServerSocket(portNumber);


		//keep the program running until the user manually exits
		while(true) 
		{
			//create socket with a connection to client
			Socket clientSocket = startUpConnection(socketServer);

			//set up for sending out messages and reading incoming messages
			PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true);
			BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));

			//hold conversation between client and server
			communicateWithClient(in, out);

			clientSocket.close();
			System.out.println("Connection with client has terminated");
		} 
	}



	// Function: startUpConnection()
	//
	// Description: This will wait for and accept a connection from the client user and 
	//    return the established server socket.
	//
	// Pre-conditions: Requires a ServerSocket to be created with the given port number
	//    and passed into the function. 
	//
	// Post conditions: A Socket with connection to the client will be created and returned
	//    back to main(). Output will be displayed confirming that a connection has been
	//    created.
	public static Socket startUpConnection(ServerSocket socket) throws IOException 
	{
		//wait for client to connect
		System.out.println("Waiting for client connection");
		Socket clientSocket = socket.accept();
		System.out.println("Connected!");

		return clientSocket;
	}



	// Function: communicateWithClient()
	//
	// Description: This will create the space for the two users to send messages back
	//    and forth. It will run until someone quits.
	//
	// Pre-condition: Requires a ServerSocket object to create a Socket object for 
	//    connection with a client. It also requires PrintWriter and BufferedReader 
	//    objects created and passed in as arguments.
	//
	// Post conditions: Does not alter any of the objects passed in to it. It will
	//    create a space for the two users to chat. Nothing will be returned.
	public static void communicateWithClient(BufferedReader in, PrintWriter out) throws IOException 
	{
		boolean didClientQuit = false;
		boolean didServerQuit = false;

		//alternate reading and sending messages while both users are connected
		while(didClientQuit == false && didServerQuit == false) 
		{

			//check return values to see if anyone quit
			didClientQuit = readMessage(in);
			if(didClientQuit) {
				break;
			}

			didServerQuit = sendMessage(out);
			if(didServerQuit) {
				break;
			}
		}
	}



	// Function: readMessage()
	// 
	// Description: Lets user receive and display a message sent from the host server
	//
	// Pre-conditions: Requires a ServerSocket to be created and connected
	//    to a requested client Socket. It also requires a BufferedReader object
	//    passed in to read the client message from.
	//
	// Post conditions: Does not alter any of the variables passed to. it will
	//    read an incoming message from the user via the client socket and display it.
	//    It will return a boolean variable stating whether or not the server quit.
	public static boolean readMessage (BufferedReader in) throws IOException 
	{
		//store incoming message in BufferedReader in object
		String newMessage;
	 	newMessage = in.readLine();


	 	//print message only if it contains characters
		if (newMessage != null) 
		{
			System.out.println(newMessage);
		}

		//let communicateWithClient function know the client has quit
		if (newMessage.contains("\\quit"))
		{
			return true;
		}

		return false;
	}



	// Function: sendMessage()
	// 
	// Description: This allows the user to input a string and send it to the host
	//   server.
	//
	// Pre-conditions: This function requires a ServerSocket to be created and
	//   connected to a requested client Socket. It also requires a PrintWriter object 
	//   passed in to stream to.
	//
	// Post conditions: Does not alter any of the objects or variables passed to it. A
	//   message will be sent to the client via the socket. A boolean variable will be 
	//   returned stating whether or not the client quit. 
	//
	// Credit: Parts of this function were taken from the oracle documents link provided in 
	//   project1.pdf. Mainly the creation of the BufferedReader in object and the
	//   statement sending the message out. 
	public static boolean sendMessage(PrintWriter out) throws IOException 
	{
		String serverHandle = "server> ";
		String updatedInput;
		System.out.print(serverHandle);
		BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
		String finalMessage = new String("");
		
		//take user input
		String userInput = in.readLine();

		// check if input is within 500 character limit, truncate if not
		if(userInput.length() > 500){
			updatedInput = userInput.substring(0, 500);
		} else {
			updatedInput = userInput;
		}

		//build final message to be sent to client
		finalMessage += serverHandle;
		finalMessage += updatedInput;

		//send printed statement to client via BufferedReader out
		out.println(finalMessage);

		//check if message includes the \quit command
		if(finalMessage.contains("\\quit")){
			return true;
		}


		return false;
	}
}