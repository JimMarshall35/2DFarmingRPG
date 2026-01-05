#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
using namespace std;

#define MAX_STRING_LENGTH 100

int main(int argc, char *argv[])
{
  if (argc< 3)
  {
     cout<<"Please pass the ip adress and port number on which the server is listening."<<endl;
     exit(-1);
  }
  
  sockaddr_in server;
  int sock_fd;

  if ((sock_fd= socket(AF_INET, SOCK_DGRAM, 0))==-1)
  {
     perror("socket: ");
     exit(-1);
  }

  server.sin_family= AF_INET;
  server.sin_port= htons(atoi(argv[2]));
  server.sin_addr.s_addr= inet_addr(argv[1]);;
  bzero(&server.sin_zero, 8);
  
  char messageBuffer[MAX_STRING_LENGTH+1]="";
  char serverReply[MAX_STRING_LENGTH+1]="";
  int len=0;

  while (true)
  { 
     cout<<"Please enter the input to send to the server: "<<endl;
     cin.getline(messageBuffer, MAX_STRING_LENGTH);

     if (strcmp(messageBuffer, "exit")==0)
      break;
    
     if (sendto(sock_fd, (void*) messageBuffer, (size_t) strlen(messageBuffer)+1, 0, (sockaddr *) &server, sizeof(server))==-1)
     {
      perror("sendto: ");
      continue;
     } 

     if (recvfrom(sock_fd, (void*) serverReply, (size_t) (MAX_STRING_LENGTH+1), MSG_WAITALL, (sockaddr *) &server, (socklen_t*) &len)==-1)
     {
      perror("recvfrom:");
      continue;
     }
 
     cout<<"The server sent: "<<serverReply<<endl<<endl;

  }//end of while loop

  close(sock_fd);
  exit(0);
}
