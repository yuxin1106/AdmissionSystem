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
#include <string>

#define admissionPORT "3676"     // the port users will be connected to 
#define BACKLOG 10               // how many pending connections queue will hold

const char* student1UDP = "21776";
const char* student2UDP = "21876";
const char* student3UDP = "21976";
const char* student4UDP = "22076";
const char* student5UDP = "22176";
const char* departAUDP = "21476";
const char* departBUDP = "21576";
const char* departCUDP = "21676";


struct Program{
     std::string programName;
     std::string departmentName;
     double GPA;
};
Program programArray[9];

void loadArray(){
   std::string fileName;
   for (int i=0; i<3; i++){
       if (i == 0){
          fileName = "recDepartmentA.txt";
       }else if(i == 1){
          fileName = "recDepartmentB.txt";
       }else{
          fileName = "recDepartmentC.txt";
       }
       std::ifstream fileInput(fileName.c_str());
       if (fileInput.is_open()){
            std::string line;
            std::getline(fileInput, line);
            programArray[3*i].programName = line.substr(2,2);
            programArray[3*i].GPA = atof((line.substr(5,3)).c_str());
            for (int j=1; j<3; j++){
                std::string line;
                std::getline(fileInput, line);
                std::size_t found = line.find_first_of("#");
                programArray[3*i+j].programName= line.substr(0, found);
                programArray[3*i+j].GPA = atof((line.substr(found+1)).c_str());          
            }
        }else{
                std::cout << "Error: " << fileName << " not found " << std::endl;
        }    
   }
}

/*void printArray(){
      for (int i=0; i<9; i++){
           std::cout << programArray[i].programName << programArray[i].GPA << std::endl;
      }
}*/

int search_compare(std::string& programName, double& GPA){
    int result = 2; // not valid program
    for (int i=0; i<9; i++){
          if (!(programArray[i].programName).compare(programName)){
              if (GPA >= programArray[i].GPA){
                  result = 1;
                  return result;
              }else{
                  result = 0;
                  return result;
              }
          }
    }
    return result;
}

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int main(void){
    struct addrinfo hints_1, *servinfo_1, *p_1;
    int status_1, socketDescriptor_1, newsocketDescriptor_1;
    char ipstr_1[INET6_ADDRSTRLEN];
    char s_1[INET6_ADDRSTRLEN];
    int yes_1 = 1;
    struct sigaction sa;
    struct sockaddr_storage their_addr_1;    // connector's address information
    int request_1 = 0;                       // connection request
    socklen_t sin_size_1;
    
    memset(&hints_1,0,sizeof hints_1);
    hints_1.ai_family = AF_UNSPEC;           // IPv4 or IPv6
    hints_1.ai_socktype = SOCK_STREAM;       // TCP stream sockets
  //hints.ai_flags = AI_PASSIVE;           // fill in IP for me 
    
    if ((status_1 = getaddrinfo("nunki.usc.edu","3676", &hints_1, &servinfo_1))!=0){
            std::cout << "server getaddrinfo failure" << std::endl;
            exit(0);
    }
    //std::cout << status << std::endl;    0
    //std::cout << servinfo->ai_addr << std::endl;     0x22210
   
    // loop through all the results and bind to the first we can 
    for (p_1 = servinfo_1; p_1!= NULL; p_1 = p_1->ai_next){
        if ((socketDescriptor_1 = socket(servinfo_1->ai_family, servinfo_1->ai_socktype, servinfo_1->ai_protocol)) == -1){
            perror("server: socket");
            continue;
        }
        if (setsockopt(socketDescriptor_1,SOL_SOCKET, SO_REUSEADDR, &yes_1, sizeof(int)) == -1){
            perror("setsockopt");
            exit(1);
        }
        if (bind(socketDescriptor_1, servinfo_1->ai_addr, servinfo_1->ai_addrlen) == -1){
            close(socketDescriptor_1);
            perror("server: bind");
            continue;
        }
        // display IP address
        void *addr;
        // get the pointer to the address itself
        // different fields in IPv4 and IPv6
        if (p_1->ai_family == AF_INET){
              struct sockaddr_in *ipv4 = (struct sockaddr_in *)p_1->ai_addr;
              addr = &(ipv4->sin_addr);
        }else{
              struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p_1->ai_addr;
              addr = &(ipv6->sin6_addr);
        }
        inet_ntop(p_1->ai_family, addr, ipstr_1, sizeof ipstr_1);
        std::cout << "The admission office has TCP port: " << admissionPORT << " and IP address: " << ipstr_1 << std::endl;
        
        break; // if we get here, we must have connected successfully
    }
    
    if (p_1 == NULL){
        std::cout << "server bind failure" << std::endl;
        return 2;
    }
    
    freeaddrinfo(servinfo_1); // all done with this structure
    
    if (listen(socketDescriptor_1,BACKLOG) == -1){
        perror("listen");
        exit(0);
    }
    //signal(SIGCHLD, SigCatcher);
    
    //std::cout << "server: waiting for connections...." << std::endl;
    // three different TCP connections to admission
    
    
    while(request_1 < 3){
         sin_size_1 = sizeof their_addr_1;
         newsocketDescriptor_1 = accept(socketDescriptor_1, (struct sockaddr *)&their_addr_1, &sin_size_1);
         if (newsocketDescriptor_1 == -1){
            perror("accept");
            exit(0);
         }
         inet_ntop(their_addr_1.ss_family, get_in_addr((struct sockaddr *)&their_addr_1), s_1, sizeof s_1);
         
         
         // create one child process for each client
         if (!fork()){  // this is the child process
            close(socketDescriptor_1);  // child doesn't need the listener
            int numbytes_1;
            //char* buffer = (char*)std::malloc(1024);   // allocate memory size for buffer
            char *buffer = new char[23];
            char *departmentName;                          // department name from received bytes
            
            if ((numbytes_1 = recv(newsocketDescriptor_1, buffer, 23, 0)) == -1){
                 perror("recv");
                 exit(1);
            }
            
            close(newsocketDescriptor_1);   
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
          
            delete[] buffer;
            std::cout << "Received the program list from <Department" << departmentName << ">" << std::endl;
            exit(0);
         }
         // close child socket in parent 
         close(newsocketDescriptor_1);
         request_1++;
    }
    close(socketDescriptor_1);
    
   int status1;
   for (int i=0; i<3; i++){
    wait(&status1);
   }
   
    std::cout << "End of Phase 1 for the admission office" << std::endl;    
    
    // load Program array with received programs
    loadArray();
    //printArray();
    
    // Phase 2 TCP connection with student
    struct addrinfo hints, *servinfo, *p;
    int status, socketDescriptor, newsocketDescriptor;
    int yes = 1;
    char ipstr[INET6_ADDRSTRLEN];
    char s[INET6_ADDRSTRLEN];
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
    
    for (p = servinfo; p!=NULL; p = p->ai_next){
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
         if (servinfo->ai_family == AF_INET){
              struct sockaddr_in *ipv4 = (struct sockaddr_in *)servinfo->ai_addr;
              addr = &(ipv4->sin_addr);
         }else{
              struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)servinfo->ai_addr;
              addr = &(ipv6->sin6_addr);
         }
         inet_ntop(servinfo->ai_family, addr, ipstr, sizeof ipstr);
         std::cout << "The admission office has TCP port: " << admissionPORT << " and IP address: " << ipstr << std::endl;
         
         break;  // if we get here, we must have connected successfully
         
         if (p == NULL){
              std::cout << "server bind failure" << std::endl;
              return 2; 
         }
    }
    
    freeaddrinfo(servinfo); // all done with this structure
    
    if (listen(socketDescriptor,BACKLOG) == -1){
        perror("listen");
        exit(0);
    }
    
    while (request < 5){
         sin_size = sizeof their_addr;
         newsocketDescriptor = accept(socketDescriptor, (struct sockaddr *)&their_addr, &sin_size);
         if (newsocketDescriptor == -1){
            perror("accept");
            exit(0);
         }
         if (!fork()){
             close(socketDescriptor);
             int numbytes;
             char* buffer = new char[50];
             std::string studentNumber;
             double GPA;
             std::string interest;
            
                         
             if ((numbytes = recv(newsocketDescriptor, buffer, 50, 0)) == -1){
                 perror("recv");
                 exit(1);
             }
            
            // convert char* to string
            std::string bufferStr = std::string(buffer);
            //std::cout << bufferStr << std::endl;
            
            std::size_t indexStudent = bufferStr.find_first_of("#");
            studentNumber = bufferStr.substr(0,indexStudent);
            std::cout << "Admission office receive the application from <Student" << studentNumber << ">" << std::endl;
            
            int compareResult = 2; // initialize to invalid application from student
            std::size_t index = bufferStr.find_first_of(":");
            std::string GPAStr = bufferStr.substr(index+1,3);
            GPA = atof((bufferStr.substr(index+1,3)).c_str());
            //std::cout << GPA << std::endl;
            for (int i=0; i<3; i++){
                index = bufferStr.find_first_of(":",index+1);
                interest = bufferStr.substr(index+1,2);
                compareResult = search_compare(interest, GPA);
                //std::cout << studentNumber << ":" << compareResult << std::endl;
                if (compareResult == 1){
                   break;
                }
            }
           
           // send acknowledgment to student 
           if (compareResult == 2){
               send(newsocketDescriptor, "0", 1, 0);  // invalid application
           }else{
               send(newsocketDescriptor,"valid",5,0);
           }
            // close socket;
            close(newsocketDescriptor); 
             
            if (compareResult == 0){
                interest = "Reject";
            }
            
            //std::cout << interest << std::endl; 
            std::string outputFile;
            if (studentNumber == "1"){ 
                  outputFile = "recStudent1.txt";
            }else if (studentNumber == "2"){
                  outputFile = "recStudent2.txt";
            }else if (studentNumber == "3"){
                  outputFile = "recStudent3.txt";
            }else if (studentNumber == "4"){
                  outputFile = "recStudent4.txt";
            }else{
                  outputFile = "recStudent5.txt";
            }
             
             std::ofstream fileOutput;
             fileOutput.open(outputFile.c_str());
             if (fileOutput.is_open()){
                 fileOutput << studentNumber + "#" + GPAStr + "#" + interest << std::endl;
                 fileOutput.close();
             }else{
                 std::cout << "unable to open file" << std::endl;
             }
             
             
             
             
          
             
           delete[] buffer;
           
           exit(0);
         }
        
         close(newsocketDescriptor);
         request++;
        
        
    }
     close(socketDescriptor);
    
    
    wait(&status1);
    
    
    
   
    // Phase2 UDP
for (int i=0; i<8; i++){
   if (!fork()){
	  const char* serverPort;
	  std::string department;
	  std::string replyFile;
	  
	  if (i == 0){
		 serverPort = student1UDP;
		 replyFile = "repStudent1.txt";
	  }else if (i == 1){
		 serverPort = student2UDP;
		 replyFile = "repStudent2.txt";
	  }else if (i == 2){
		 serverPort = student3UDP;
		 replyFile = "repStudent3.txt";
	  }else if (i == 3){
	     serverPort = student4UDP;
	     replyFile = "repStudent4.txt";
	  }else if (i == 4){
	     serverPort = student5UDP;
	     replyFile = "repStudent5.txt";
	  }else if (i == 5){
	     serverPort = departAUDP;
	  }else if (i == 6){
	     serverPort = departBUDP;
	  }else if (i == 7){
	     serverPort = departCUDP;
	  }

       std::ifstream readReply;
       readReply.open(replyFile.c_str());
       std::string replyLine;
       std::getline(readReply, replyLine);
       //std::cout << replyLine << std::endl;
       if (!replyLine.compare("1")){
            exit(0);   // exit child process, the student do not establish UDP connection to admission
        }
       readReply.close();
       
      
	   int socketDescriptor_2;
	   struct addrinfo hints_2, *admissioninfo, *p_2;
	   int rv_2, numbytes_2;
	   struct sockaddr_storage their_addr_2;
	   socklen_t addr_len_2;
	   char ipstr_2[INET6_ADDRSTRLEN];
	   char s_2[INET6_ADDRSTRLEN];
       struct sockaddr_in sin;
       socklen_t sin_len;
   
	   memset(&hints_2, 0, sizeof hints_2);
	   hints_2.ai_family = AF_UNSPEC;
	   hints_2.ai_socktype = SOCK_DGRAM;
	   //hints_2.ai_flags = AI_PASSIVE;
   
	   if ((rv_2 = getaddrinfo("nunki.usc.edu", serverPort, &hints_2, &admissioninfo))!=0){
		   std::cout << "student getaddrinfo failure" << std::endl;
		   exit(0);
	   } 
   
	   for (p_2 = admissioninfo; p_2 != NULL; p_2 = p_2->ai_next){
		   if ((socketDescriptor_2 = socket(p_2->ai_family, p_2->ai_socktype, p_2->ai_protocol)) == -1){
			  perror("student: socket");
			  continue;
		   }
		   
		  
		   //inet_ntop(admissioninfo->ai_family, get_in_addr((struct sockaddr *)admissioninfo->ai_addr), s_2, sizeof s_2);
		   break;

	   }
	   if (p_2 == NULL){
			  std::cout << "admission: failed to bind socket" << std::endl;
			  return 2;
	   }
	  inet_ntop(admissioninfo->ai_family, get_in_addr((struct sockaddr *)admissioninfo->ai_addr), s_2, sizeof s_2);
    
       // open recStudent#.txt files to 
       std::string fileName;
       
       if (i < 5){ 
          if (i == 0){
              fileName = "recStudent1.txt";
          }else if (i == 1){
              fileName = "recStudent2.txt";
          }else if (i == 2){
              fileName = "recStudent3.txt";
          }else if (i == 3){
              fileName = "recStudent4.txt";
          }else if (i == 4){
              fileName = "recStudent5.txt";
          }
          
          std::ifstream fileInput(fileName.c_str());
          std::string line;
          std::getline(fileInput,line);
          std::string reject = line.substr(6,6);
         // std::cout << reject << std::endl;
         // std::cout << reject.compare("Reject") << std::endl;
          
          if (!reject.compare("Reject")){
              if (sendto(socketDescriptor_2, "Reject", 6, 0, p_2->ai_addr, p_2->ai_addrlen) == -1){
              // sendto(socketDescriptor_2, "Reject", 6, 0, (struct sockaddr*)&their_addr_2, addr_len_2)  invalid argument
                 perror("admission send failure");
                 exit(1);
              }
              std::cout << "The admission office has send the application result to <Student" << i + 1 << ">" << std::endl;
          }else{
			  std::size_t index = line.find_first_of("#");
			  index = line.find_first_of("#", index+1);
			  std::string programName = line.substr(index+1,2);
			  std::string departmentName = line.substr(index+1,1);
			  std::string packetStr = "Accept#" + programName + "#" + "department" + departmentName;
			 // std::cout << packetStr << std::endl;
			  if (sendto(socketDescriptor_2, packetStr.c_str(), strlen(packetStr.c_str()), 0, p_2->ai_addr, p_2->ai_addrlen) == -1){
			  // sendto(socketDescriptor_2, packetStr.c_str(), strlen(packetStr.c_str()), 0, (struct sockaddr*)&their_addr_2, addr_len_2)
			     perror("admission send failure");
			     exit(1);
			  }
			  std::cout << "The admission office has send the application result to <Student" << i + 1 << ">" << std::endl;
          }
                  
          fileInput.close();
          
          sin_len = sizeof sin;
          getsockname(socketDescriptor_2, (struct sockaddr *)&sin, &sin_len);
          std::cout << "The admission office has UDP port " << ntohs(sin.sin_port) << " and IP address " << s_2 << " for Phase 2" << std::endl;
         
          
    }else{
          if (i == 5){
             department = "A";
          }else if (i == 6){
             department = "B";
          }else if (i == 7){
             department = "C";
          }
          //std::cout << department << std::endl;
            
          for (int j=0; j<5; j++){
             std::string inputFileName; 
			  if (j == 0){
				  inputFileName = "recStudent1.txt";
			  }else if (j == 1){
				  inputFileName = "recStudent2.txt";
			  }else if (j == 2){
				  inputFileName = "recStudent3.txt";
			  }else if (j == 3){
				  inputFileName = "recStudent4.txt";
			  }else if (j == 4){
				  inputFileName = "recStudent5.txt";
			  }
             std::ifstream inputFile(inputFileName.c_str());
             std::string fileLine;
             std::getline(inputFile, fileLine);
             inputFile.close();
             std::size_t indexDepartment = fileLine.find_first_of("#");
			 indexDepartment = fileLine.find_first_of("#", indexDepartment+1);
			 
			 //std::cout << fileLine.substr(indexDepartment+1,1) << std::endl;
			 //std::cout << (fileLine.substr(indexDepartment+1,1)).compare(department) << std::endl;
			 
			 if (!(fileLine.substr(indexDepartment+1,1)).compare(department)){
			    std::string packetDepartStr = "Student" + fileLine;
			   // std::cout << packetDepartStr << std::endl;
			    sendto(socketDescriptor_2, packetDepartStr.c_str(), strlen(packetDepartStr.c_str()), 0, p_2->ai_addr, p_2->ai_addrlen);
			    std::cout << "The admission office has send one admitted student to <Department" << department << ">" << std::endl;
			 }
          }
          
          sin_len = sizeof sin;
          getsockname(socketDescriptor_2, (struct sockaddr *)&sin, &sin_len);
          std::cout << "The admission office has UDP port " << ntohs(sin.sin_port) << " and IP address " << s_2 << " for Phase 2 " << std::endl;
          
    }
       
   
   
   
    freeaddrinfo(admissioninfo);
    close(socketDescriptor_2);
    
    
    exit(0);
    }
}
  
      for (int i=0; i<20; i++){
           wait(&status1);
      }
      std::cout << "End of Phase 2 for the admission office" << std::endl;

  
    
    
   
    
    return 0;
}
