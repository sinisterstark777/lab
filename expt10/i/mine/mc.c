#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<string.h>
#define sizee 20
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
	struct sockaddr_in saddr; 
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int slen = sizeof(saddr);
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	connect(sockfd, (struct sockaddr*)&saddr, slen);
	char buffer[20];
	strcpy(buffer,"Connected");
	send(sockfd, buffer, sizeof(buffer), 0);
	printf("connected\n");
	//----------------------------------------------------------

	struct packet pack;
	struct packet spack[20];
	int arr[100], size, ack, wsize, flag=0;
	printf("Enter array size : ");
	scanf("%d", &size);
	printf("Enter window size : ");
	scanf("%d", &wsize);
	send(sockfd, &size, sizeof(size), 0);
	send(sockfd, &wsize, sizeof(wsize), 0);
	printf("Enter array : ");
	for(int i=0; i<size; i++)
	{
		scanf("%d", &spack[i].data);
		spack[i].ack = 1;
		spack[i].index = i;
	}

	flag = 0;
	for(int i=0; i<size+wsize; i++)
	{
		if(flag<wsize)
		{
			send(sockfd, &spack[i], sizeof(pack), 0);
			printf("\nsend : %d", spack[i].data);
			flag++;
			printf("\nindex : %d, ack : %d, data : %d", spack[i].index, spack[i].ack, spack[i].data);
		}
		else
		{
			recv(sockfd, &pack, sizeof(pack), 0);
			if(pack.ack == 0)
			{
				printf("Failed to transmit %d", spack[pack.index].data);
				spack[pack.index].ack = 1;
				send(sockfd, &spack[pack.index], sizeof(pack), 0);
				printf("\nsend : %d", spack[pack.index].data);
				sleep(1);
				i--;
			}
			else
			{
				if(((pack.index) + wsize) < size)
				{
					send(sockfd, &spack[(pack.index)+wsize], sizeof(pack), 0);
					printf("\nsend : %d", spack[(pack.index)+wsize].data);
					printf("\nindex : %d, ack : %d, data : %d", spack[(pack.index)+wsize].index, spack[(pack.index)+wsize].ack, spack[(pack.index)+wsize].data);
				}
			}
		}		
	}
	
	close(sockfd);
}
