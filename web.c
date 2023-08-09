#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX 1000000

int main()
{
    char buffer[MAX], bufferNew[MAX], outputBuffer[MAX];
    char str[] = "secondary.biharboardonline.com";
    int indexBuf = 0, indexBufNew = 0, indexStr = 0, replaced = 0;

 int sockfd = socket(AF_INET, SOCK_STREAM, 0);
 if(sockfd == -1)
 {
  printf("Socket creation failed!\n");
  return -1;
 }
 printf("Socket created\n");
 
 struct sockaddr_in server;
 server.sin_addr.s_addr = inet_addr("127.0.0.1");
 server.sin_family = AF_INET;
 server.sin_port = htons(7255);
 
 if (bind(sockfd, (struct sockaddr*)&server,sizeof(server)) < 0)
 {
  printf("Binding failed!\n");
  return -1;
 }
 
 printf("Binding successful\n");
 
 if (listen(sockfd, 1) < 0)
 {
  printf("Listening failed\n");
  return -1;
 }
 
 printf("Listening...\n");
 
 int size = sizeof(server);
 int client = accept(sockfd, (struct sockaddr*)&server,(socklen_t *)&size);
 if (client < 0)
 {
  printf("Acceptance error!\n");
  return -1;
 }
 printf("Client accepted\n");

 recv(client, buffer, sizeof(buffer), 0);
    printf("Buffer = %s\n", buffer);
    
    while(buffer[indexBuf] != '\0')
    {
        if (buffer[indexBuf] == ':' && !replaced)
        {
            replaced = 1;

            bufferNew[indexBufNew++] = buffer[indexBuf++];
            bufferNew[indexBufNew++] = buffer[indexBuf++];
        
            while(str[indexStr]!='\0')
                bufferNew[indexBufNew++] = str[indexStr++];

            while(buffer[indexBuf]!='\n')
                indexBuf++;     
        }

        bufferNew[indexBufNew++] = buffer[indexBuf++];
    }
    bufferNew[indexBufNew] = '\0';

    printf("Buffer\n%s", buffer);
    printf("Buffernew\n%s", bufferNew);
    

    int sockfdNew = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfdNew == -1)
 {
  printf("Socket creation failed!\n");
  return -1;
 }

    struct sockaddr_in serverNew;
 serverNew.sin_addr.s_addr = inet_addr("115.243.18.40");
 serverNew.sin_family = AF_INET;
 serverNew.sin_port = htons(80);

    if (connect(sockfdNew, (struct sockaddr *)&serverNew, sizeof(serverNew)) < 0)
 {
  printf("Connection error\n");
  return -1;
 }
 printf("Connected to server\n");
    send(sockfdNew, bufferNew, sizeof(bufferNew), 0);
 recv(sockfdNew, outputBuffer, sizeof(outputBuffer), 0);

 printf("Output Buffer\n%s", outputBuffer);
 send(client, outputBuffer, sizeof(outputBuffer), 0);

 return 0;
}
