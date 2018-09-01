Full Name: Xin Yu
Student ID: 6683105376

For Phase 1 I have two cpp files: Admission.cpp, Department.cpp to transfer program information from departments to admission.

Admission.cpp: It first use getaddrinfo function to save the ip information for later representation on the screen and to create socket. The socket is then created and binds to the server address. The server socket listens to incoming 3 connections from departments. In a while loop, it accepts the three connections by creating a child socket and fork a child process for each connections. In the child process, it opens a txt file to save the received information for further process. 

Department.cpp: It first gets the server ip address and port number to create socket to connect to the server. It then forks 3 child processes for 3 departments to create socket, connect to server, and send department information read from respective txt files. 

To compile type:
g++ -o Admission Admission.cpp -lsocket -lnsl -lresolv
g++ -o Department Department.cpp -lsocket -lnsl -lresolv

To run:
open two terminal
./Admission in a terminal
./Department in another terminal

Occasionally if you just transferred and compiled the files, the first run is not complete on the department side. But the second time and later ones are always successful. 