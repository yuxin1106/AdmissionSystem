#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string>
#include <fstream>
#include <cstddef>

#define admissionPort "3676"   // the department should know the static TCP port number, it will be connecting to 
const char* departAUDP = "21476";
const char* departBUDP = "21576";
const char* departCUDP = "21676";


void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void){
    int socketDescriptor;
    struct addrinfo hints, *servinfo;
    char s[INET6_ADDRSTRLEN];
    int rv;
    struct sockaddr_in sin;
    socklen_t sin_len;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo("nunki.usc.edu", "3676", &hints, &servinfo))!=0){
         std::cout << "server getaddrinfo failure" << std::endl;
         return 1;
    }
    
    // fork 3 child processes for 3 departments
    for (int i=0; i<3; i++){
        if (!fork()){
            if ((socketDescriptor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol))==-1){
                perror("client: socket");
                exit(1);
            }
            if (connect(socketDescriptor,servinfo->ai_addr, servinfo->ai_addrlen) == -1){
                close(socketDescriptor);
                perror("client: connect");
                exit(1);
            }
            // get IP address of the department
            inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr), s, sizeof s);
            // get dynamic TCP after first send
            
            // get different file name
            std::string fileName;
            std::string departmentLetter;
            if (i == 0){
               fileName = "departmentA.txt";
               departmentLetter = "A";
            }else if (i == 1){
               fileName = "departmentB.txt";
               departmentLetter = "B";
            }else{
               fileName = "departmentC.txt";
               departmentLetter = "C";
            }
            
            send(socketDescriptor, departmentLetter.c_str(), 1, 0);
            send(socketDescriptor, "#", 1, 0);
            
            sin_len = sizeof sin;
            getsockname(socketDescriptor, (struct sockaddr *)&sin, &sin_len);
            std::cout << "<Department" << departmentLetter << "> has TCP port " << ntohs(sin.sin_port) << " and IP address " << s << " for Phase 1" << std::endl;;
            std::cout << "<Department" << departmentLetter << "> is now connected to the admission office " << std::endl;
            
            
            // read in file
            std::ifstream fileInput(fileName.c_str());
            if (fileInput.is_open()){
                for (int j=0; j<3; j++){
                    std::string line;
                    std::getline(fileInput, line);
                    std::size_t found = line.find_first_of("#");
                    std::string programName = line.substr(0, found);
                    
                    //std::cout << strlen(line.c_str()) << std::endl;
                    //std::cout << line << " " << line.c_str() << std::endl;
                    send(socketDescriptor, line.c_str(), strlen(line.c_str()), 0);
                    send(socketDescriptor, "\n", 1, 0);
                    
                    std::cout << "<Department" << departmentLetter << "> has sent <"<< programName << "> to the admission office" << std::endl;
                }
            }else{
                std::cout << "Error: " << fileName << " not found " << std::endl;
            }
            
            // close input file
            fileInput.close();
            // close socket
            close(socketDescriptor);
              
            std::cout << "Updating the admission office is done for <Department" << departmentLetter << ">" << std::endl;
            std::cout << "End of Phase 1 for <Department" << departmentLetter << ">" << std::endl;
            
            exit(0); // exit child process   
            
        }    
    }

    freeaddrinfo(servinfo); // all done with the structure

    
    int status1;
    for (int i=0; i<3; i++){
        wait(&status1);
    }
    
    // phase 2 UDP
    for (int i=0; i<3; i++){
   if (!fork()){
	  const char* departUDP;
	  std::string departmentName;
	  
	  if (i == 0){
		 departUDP = departAUDP;
		 departmentName = "A";
	  }else if (i == 1){
		 departUDP = departBUDP;
		 departmentName = "B";
	  }else{
		 departUDP = departCUDP;
		 departmentName = "C";
	  }


	   int socketDescriptor_2;
	   struct addrinfo hints_2, *departinfo, *p_2;
	   int rv_2, numbytes_2;
	   struct sockaddr_storage their_addr;
	   socklen_t addr_len;
	   char ipstr_2[INET6_ADDRSTRLEN];
	   char s_2[INET6_ADDRSTRLEN];
   
   
	   memset(&hints_2, 0, sizeof hints_2);
	   hints_2.ai_family = AF_UNSPEC;
	   hints_2.ai_socktype = SOCK_DGRAM;
   
	   if ((rv_2 = getaddrinfo("nunki.usc.edu", departUDP, &hints_2, &departinfo))!=0){
		   std::cout << "department getaddrinfo failure" << std::endl;
		   exit(0);
	   } 
   
	   for (p_2 = departinfo; p_2 != NULL; p_2 = p_2->ai_next){
		   if ((socketDescriptor_2 = socket(p_2->ai_family, p_2->ai_socktype, p_2->ai_protocol)) == -1){
			  perror("department: socket");
			  continue;
		   }
		   
		   if (bind(socketDescriptor_2, p_2->ai_addr, p_2->ai_addrlen) == -1){
		      close(socketDescriptor_2);
		      perror("department: socket");
		      continue;
		   }
		   // display IP address
		   void *addr;
			// get the pointer to the address itself
			// different fields in IPv4 and IPv6
		   if (p_2->ai_family == AF_INET){
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)p_2->ai_addr;
				addr = &(ipv4->sin_addr);
		   }else{
				struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p_2->ai_addr;
				addr = &(ipv6->sin6_addr);
		   }
		   inet_ntop(p_2->ai_family, addr, ipstr_2, sizeof ipstr_2);
		   std::cout << "<Department" << departmentName << "> has UDP port: " << departUDP << " and IP address: " << ipstr_2 << " for Phase 2 " << std::endl;

		   break;

	   }
	   if (p_2 == NULL){
			  std::cout << "department: failed to bind socket" << std::endl;
			  return 2;
	   }
	   freeaddrinfo(departinfo);
    
    char* buffer = new char[32];
    // multiple UDP messages are not concatenated
    addr_len = sizeof their_addr;
    while(1){
		if ((numbytes_2 = recvfrom(socketDescriptor_2, buffer, 32, 0, (struct sockaddr*)&their_addr, &addr_len)) == -1){
			perror("recvfrom");
			exit(1);    
		}
    
		std::string bufferStr = std::string(buffer);
		//std::cout << bufferStr << std::endl;
	
		std::size_t indexStudent = bufferStr.find_first_of("#");
		std::string studentNumber = bufferStr.substr(indexStudent-1,1);
		std::cout << "<Student" << studentNumber << "> has been admitted to <Department"<< departmentName << ">" << std::endl; 
		std::cout << "End of Phase 2 for <Department" << departmentName << ">" << std::endl;
    }
    
    std::cout << "End of Phase 2 for <Department" << departmentName << ">" << std::endl;
    delete[] buffer;
   
    close(socketDescriptor_2);
    //std::cout << "End of Phase 2 for <Department" << departmentName << ">" << std::endl;
    //sleep(1000);
    //exit(0);
     return 0;
    }
}

    
    
    
    

return 0;
}
