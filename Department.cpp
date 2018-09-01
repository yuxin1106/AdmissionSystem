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
    wait(&status1);
    

return 0;
}
