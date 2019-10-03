This program will implement a simple file transfer system utilizing the TCP protocol. The client will be written in Python, while the server will be written in C. For more information on this project, see the Project2.pdf file in this repository.




The general format for running ftserver and ftclient are as follows:


ftserver
	To compile, you can enter: gcc -o ftserver ftserver.c OR you can enter "make compileserv" to utilize my makefile rule

	To run, you have two options. You can enter: ./ftserver <port number>  OR you can enter "make runserv" to utilize my makefile rule. This will execute the command "./ftserver 1031", so it will run on port 1031.


ftclient
	No compilation need



	To run, there are two formats depending on the command you want to run:

	1. For "-l" --> python ftclient.py <server host> <server port> <command> <data port>
	An alternative is to just type "runclient", which will run my provided bash file that matches the port for my ftserver makefile rules.

	For "-g" --> python ftclient.py <server host> <serv port> <command> <file name> <data port>
	An alternative is to just type "runclientfile", another bash executable file that matches the necessary server parameters in my makefile rules above. 


	If you'd like to use my bash executables, you can just change the commands/port numbers/file names as needed. The programs should still work as long as the server port on each matches. Please open the bash files if you'd like to see the specific command line parameters used in each command. 


******IMPORTANT NOTES********

1. ftserver MUST be ran on flip1 since the IP address is hardcoded into the ftclient file. Thus, the <server host> command line parameter must be "flip1.engr.oregonstate.edu", not "flip1" or any of the other server names.

2. ftclient MUST be ran on flip2 since the IP address is hardcoded into the ftserver file. 

3. If you're transferring a file that already exists in the client's current directory, you'll be prompted with a message asking if you'd like to overwrite. Given the timing of the program, you'll have to enter "y" and "enter" quickly to avoid a connection error on the server program for the data connection (If you take too long, the server program will try to connect to a not yet created socket by the client). Also, if you enter "n" instead to avoid an overwrite, then this will lead to a "broken pipe" error on the server program. This is the only instance where the server will exit and not wait for another connection. However, entering "n" will lead to the client exiting the program correctly and not overwriting the file since the user chose not to.
