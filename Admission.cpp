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
#include <signal.h>

#define admissionPORT "3676"     // the port users will be connected to 
#define BACKLOG 10               // how many pending connections queue will hold

struct Program{
     char* programName;
     char* departmentName;
     double GPA;
};

Program programArray[9];
//void *SigCatcher(int n){
  //  wait3(NULL, WNOHANG, NULL);
//}

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void){
    struct addrinfo hints, *servinfo, *p;
    int status, socketDescriptor, newsocketDescriptor;
    char ipstr[INET6_ADDRSTRLEN];
    char s[INET6_ADDRSTRLEN];
    int yes = 1;
    struct sigaction sa;
    struct sockaddr_storage their_addr;    // connector's address information
    int request = 0;                       // connection request
    socklen_t sin_size;
    
    memset(&hints,0,sizeof hints);
    hints.ai_family = AF_UNSPEC;           // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;       // TCP stream sockets
  //hints.ai_flags = AI_PASSIVE;           // fill in IP for me 
    
    if ((status = getaddrinfo("nunki.usc.edu","3676", &hints, &servinfo))!=0){
            std::cout << "server getaddrinfo failure" << std::endl;
            exit(0);
    }
    //std::cout << status << std::endl;    0
    //std::cout << servinfo->ai_addr << std::endl;     0x22210
   
    // loop through all the results and bind to the first we can 
    for (p = servinfo; p!= NULL; p = p->ai_next){
        if ((socketDescriptor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }
        if (setsockopt(socketDescriptor,SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if (bind(socketDescriptor, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
            close(socketDescriptor);
            perror("server: bind");
            continue;
        }
        // display IP address
        void *addr;
        // get the pointer to the address itself
        // different fields in IPv4 and IPv6
        if (p->ai_family == AF_INET){
              struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
              addr = &(ipv4->sin_addr);
        }else{
              struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
              addr = &(ipv6->sin6_addr);
        }
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        std::cout << "The admission office has TCP port: " << admissionPORT << " and IP address: " << ipstr << std::endl;
        
        break; // if we get here, we must have connected successfully
    }
    
    if (p == NULL){
        std::cout << "server bind failure" << std::endl;
        return 2;
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (listen(socketDescriptor,BACKLOG) == -1){
        perror("listen");
        exit(0);
    }
    //signal(SIGCHLD, SigCatcher);
    
    //std::cout << "server: waiting for connections...." << std::endl;
    // three different TCP connections to admission
    
    
    while(request < 3){
         sin_size = sizeof their_addr;
         newsocketDescriptor = accept(socketDescriptor, (struct sockaddr *)&their_addr, &sin_size);
         if (newsocketDescriptor == -1){
            perror("accept");
            exit(0);
         }
         inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
         
         
         // create one child process for each client
         if (!fork()){  // this is the child process
            close(socketDescriptor);  // child doesn't need the listener
            int numbytes;
            //char* buffer = (char*)std::malloc(1024);   // allocate memory size for buffer
            char *buffer = new char[23];
            char *departmentName;                          // department name from received bytes
            char* programName;                             // program name from received bytes
            char* GPA;                                     // GPA from received bytes
            
            if ((numbytes = recv(newsocketDescriptor, buffer, 23, 0)) == -1){
                 perror("recv");
                 exit(1);
            }
            
            close(newsocketDescriptor);   
            //std::cout << buffer << std::endl;
            std::string outputFile;
            departmentName = strtok(buffer, "#");
            
            // receive three lines via three packets
            
            // write received buffer of each department to txt
            if (*departmentName == 'A'){
                outputFile = "recDepartmentA.txt";
            }else if(*departmentName == 'B'){
                outputFile = "recDepartmentB.txt";
            }else{
                outputFile = "recDepartmentC.txt";
            } 
            
            std::ofstream fileOutput;
            fileOutput.open(outputFile.c_str());
            if (fileOutput.is_open()){
                fileOutput.write(buffer,23);
                fileOutput.close();
            }else{
                std::cout << "unable to open file" << std::endl;
            }
           /*for (int i=0; i<3; i++){
                  programName = strtok(NULL, "#");
                  //std::cout << programName << std::endl;
                  GPA = strtok(NULL, "\n");
                  //std::cout << GPA << std::endl;
                  Program receivedProgram;
                  receivedProgram.departmentName = departmentName;
                  receivedProgram.programName = programName;
                  receivedProgram.GPA = atof(GPA);
                  //std::cout << receivedProgram.programName << receivedProgram.GPA << std::endl;
                  programArray[request*3+i] = receivedProgram;
                  
                  
            }*/
            //std::cout << programArray[request*3].programName << std::endl;
            delete[] buffer;
            std::cout << "Received the program list from <Department" << departmentName << ">" << std::endl;
            exit(0);
         }
         // close child socket in parent 
         close(newsocketDescriptor);
         request++;
    }
    
   int status1;
   for (int i=0; i<3; i++){
    wait(&status1);
   }
   
    
  
    std::cout << "End of Phase 1 for the admission office" << std::endl;    

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    return 0;
}
