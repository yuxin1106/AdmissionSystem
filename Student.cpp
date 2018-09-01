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
#include <fstream>

#define admissionPort "3676" // student should know the static TCP port number, it will be connecting to
const char* student1UDP = "21776";
const char* student2UDP = "21876";
const char* student3UDP = "21976";
const char* student4UDP = "22076";
const char* student5UDP = "22176";

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
    
    // fork 5 child processes for 5 students
    for (int i=0; i<5; i++){
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
            // get IP address of the student
            inet_ntop(servinfo->ai_family, get_in_addr((struct sockaddr *)servinfo->ai_addr), s, sizeof s);
            // get dynamic TCP after first send
            
            // get different file name
            std::string fileName;
            std::string studentNumber;
            std::string replyFile;
            if (i == 0){
               fileName = "student1.txt";
               studentNumber = "1";
               replyFile = "repStudent1.txt";
            }else if (i == 1){
               fileName = "student2.txt";
               studentNumber = "2";
               replyFile = "repStudent2.txt";
            }else if (i == 2){
               fileName = "student3.txt";
               studentNumber = "3";
               replyFile = "repStudent3.txt";
            }else if (i == 3){
               fileName = "student4.txt";
               studentNumber = "4";
               replyFile = "repStudent4.txt";
            }else if (i == 4){
               fileName = "student5.txt";
               studentNumber = "5";
               replyFile = "repStudent5.txt";
            }
            
            // send student number
           // send(socketDescriptor, studentNumber.c_str(), 1, 0);
           // send(socketDescriptor, "#", 1, 0);
            
           // getsockname(socketDescriptor, (struct sockaddr *)&sin, &sin_len);
            //std::cout << "<Student" << studentNumber << "> has TCP port " << ntohs(sin.sin_port) << " and IP address " << s << std::endl;;
            
            
            // read in file to send applications in 4 packets
            std::ifstream fileInput(fileName.c_str());
            std::string stringToSend = studentNumber + "#";
           
            if (fileInput.is_open()){
                for (int j=0; j<4; j++){
                    std::string line;
                    std::getline(fileInput, line);
                    stringToSend += line + "\n";
                   // send(socketDescriptor, line.c_str(), strlen(line.c_str()), 0);
                   // send(socketDescriptor, "\n", 1, 0); 
                    //std::cout << line << std::endl;                  
                }
            }else{
                std::cout << "Error: " << fileName << " not found " << std::endl;
            }
            std::cout << "Completed sending application for <Student" << studentNumber << ">" << std::endl;
            // close input file
            fileInput.close();
            
            send(socketDescriptor, stringToSend.c_str(), strlen(stringToSend.c_str()),0);
            sin_len = sizeof sin;
            if (getsockname(socketDescriptor, (struct sockaddr *)&sin, &sin_len) == -1){
               perror("getsockname failure");
               exit(1);
            }
            std::cout << "<Student" << studentNumber << "> has TCP port " << ntohs(sin.sin_port) << " and IP address " << s << std::endl;;
            
               int numbytes;
               char* buffer = new char[5];
               if ((numbytes = recv(socketDescriptor, buffer, 5, 0)) == -1){
                  perror("recv");
                  exit(1);
               }else{
                  //getsockname(socketDescriptor, (struct sockaddr *)&sin, &sin_len);
                  //std::cout << "<Student" << studentNumber << "> has TCP port " << ntohs(sin.sin_port) << " and IP address " << s << std::endl;;
                  std::cout << "<Student" << studentNumber << "> has received the reply from the admission office" << std::endl;
               }
               //std::cout << buffer << std::endl;
               //std::cout << numbytes << std::endl;
               std::ofstream replyOutput;
               replyOutput.open(replyFile.c_str());
               replyOutput << numbytes << std::endl;
               replyOutput.close();
              
               
               delete[] buffer;
            
            // close socket
            close(socketDescriptor);
              
            
            //std::cout << "<Student" << studentNumber << "> has received the reply from the admission office" << std::endl;
            exit(0); // exit child process   
            
        }    
    }

    freeaddrinfo(servinfo); // all done with the structure
    close(socketDescriptor);
    // wait
    //int status1;
    //for (int i=0; i<3; i++){
      //   wait(&status1);
    //}

   
    
   // Phase2 UDP
for (int i=0; i<5; i++){
      //std::string studentNumber;
   if (!fork()){
      std::string studentNumber;
	  const char* studentUDP;
	  std::string replyInfo;
	  
	  if (i == 0){
		 studentUDP = student1UDP;
		 studentNumber = "1";
		 replyInfo = "repStudent1.txt";
	  }else if (i == 1){
		 studentUDP = student2UDP;
		 studentNumber = "2";
		 replyInfo = "repStudent2.txt";
	  }else if (i == 2){
		 studentUDP = student3UDP;
		 studentNumber = "3";
		 replyInfo = "repStudent3.txt";
	  }else if (i == 3){
	     studentUDP = student4UDP;
		 studentNumber = "4";
		 replyInfo = "repStudent4.txt";
	  }else{
	     studentUDP = student5UDP;
		 studentNumber = "5";
		 replyInfo = "repStudent5.txt";
	  }
  
       std::ifstream readReply;
       readReply.open(replyInfo.c_str());
       std::string replyLine;
       std::getline(readReply, replyLine);
       //std::cout << replyLine << std::endl;
       if (!replyLine.compare("1")){
            exit(0);   // exit child process, the student do not establish UDP connection to admission
       }
       readReply.close();
      
       
// declare differently above
	   int socketDescriptor_2;
	   struct addrinfo hints_2, *studentinfo, *p_2;
	   int rv_2, numbytes_2;
	   struct sockaddr_storage their_addr_2;
	   socklen_t addr_len_2;
	   char ipstr_2[INET6_ADDRSTRLEN];
	   char s_2[INET6_ADDRSTRLEN];
   
   
	   memset(&hints_2, 0, sizeof hints_2);
	   hints_2.ai_family = AF_UNSPEC;
	   hints_2.ai_socktype = SOCK_DGRAM;
   
	   if ((rv_2 = getaddrinfo("nunki.usc.edu", studentUDP, &hints_2, &studentinfo))!=0){
		   std::cout << "student getaddrinfo failure" << std::endl;
		   exit(0);
	   } 
   
	   for (p_2 = studentinfo; p_2 != NULL; p_2 = p_2->ai_next){
		   if ((socketDescriptor = socket(p_2->ai_family, p_2->ai_socktype, p_2->ai_protocol)) == -1){
			  perror("student: socket");
			  continue;
		   }
		   
		   if (bind(socketDescriptor, p_2->ai_addr, p_2->ai_addrlen) == -1){
		      close(socketDescriptor);
		      perror("student: socket");
		      continue;
		   }
		   // display IP address
		   void *addr_2;
			// get the pointer to the address itself
			// different fields in IPv4 and IPv6
		   if (p_2->ai_family == AF_INET){
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)p_2->ai_addr;
				addr_2 = &(ipv4->sin_addr);
		   }else{
				struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p_2->ai_addr;
				addr_2 = &(ipv6->sin6_addr);
		   }
		   //inet_ntop(admissioninfo->ai_family, get_in_addr((struct sockaddr *)admissioninfo->ai_addr), s_2, sizeof s_2);
		   inet_ntop(studentinfo->ai_family, addr_2, ipstr_2, sizeof ipstr_2);
		   std::cout << "<Student" << studentNumber << "> has UDP port: " << studentUDP << " and IP address: " << ipstr_2 << " for Phase 2" << std::endl;
           
		   break;

	   }
	   if (p_2 == NULL){
			  std::cout << "student: failed to bind socket" << std::endl;
			  return 2;
	   }
	   
	   freeaddrinfo(studentinfo);
    
    char* buffer = new char[23];
    addr_len_2 = sizeof their_addr_2;
    recvfrom(socketDescriptor, buffer, 23, 0, (struct sockaddr*)&their_addr_2, &addr_len_2);
    
   /* if ((numbytes_2 = recvfrom(socketDescriptor, buffer, 23, 0, (struct sockaddr*)&their_addr_2, &addr_len_2)) == -1){
        perror("recvfrom");
        exit(1);    
    }else{
        std::cout << "<Student" << studentNumber <<"> has received the application result" << std::endl;
    }*/
        
    std::cout << "<Student" << studentNumber <<"> has received the application result" << std::endl;
    
    //std::string bufferStr = std::string(buffer);
     
    /*if (bufferStr.compare("Reject")){
		std::size_t index = bufferStr.find_first_of("#"); 
		std::string departmentName = bufferStr.substr(index+1,1);
		
		std::cout << "<Student" << studentNumber << "> has been admitted to <Department" << departmentName << ">" << std::endl;
	}*/
	std::cout << "End of Phase 2 for <Student" << studentNumber << ">" << std::endl;
	
    delete[] buffer;
    close(socketDescriptor_2);
    
    //wait(NULL);
     exit(0);
    }
    
}
   
   //std::cout << "End of student" << std::endl;
   //for (int i=0; i<10; i++){
          int status1;
          wait(&status1);
      //}
     
     return 0;
}

 
