#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#define port 5060

struct packet
{
	int index;
	int data;
	int ack;
};

void main()
{
	//connection establishment
	struct sockaddr_in saddr, caddr;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int csock;
	int slen = sizeof(saddr), clen;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bind(sockfd, (struct sockaddr*)&saddr, slen);
	listen(sockfd, 5);
	csock = accept(sockfd, (struct sockaddr*)&caddr, &clen);
	char buffer[20];
	recv(csock, buffer, sizeof(buffer), 0);
	printf("%s", buffer);
	//----------------------------------------------------------
	
	int size, ack, wsize, flag=0;
	struct packet spack[20], pack;
	recv(csock, &size, sizeof(size), 0);
	recv(csock, &wsize, sizeof(wsize), 0);
	int ran = rand() % size;
 //---------------------------------------------------------
 printf("\nran : %d\n", ran);
 for(int i=0; i<size; i++)
 {
 	spack[i].ack = 1;
 }
	while(1)
	{
		recv(csock, &pack, sizeof(pack), 0);
		spack[pack.index] = pack;
		printf("\nrecv : %d", spack[pack.index].data);
		if(pack.index == ran)
		{
			printf("Failed to transmit %d", spack[pack.index].data);
			spack[pack.index].ack = 0;
			ran = -1;
		}
		send(csock, &spack[pack.index], sizeof(pack), 0);

		printf("\nindex : %d, ack : %d, data : %d", spack[pack.index].index, spack[pack.index].ack, spack[pack.index].data);
		if(pack.index + 1 == size)
			break;
	}
	
	printf("\nArray : ");
	for(int i=0; i<size; i++)
	{
		printf("%d  ", spack[i].data);
	}
	
	close(sockfd);
	close(csock);
}
