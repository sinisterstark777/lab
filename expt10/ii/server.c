#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8000
#define SIZE 100

typedef struct packet {
	int data;
	int type; // SEQ (0), ACK (1) or NACK(-1)
	int seq; // Sequence number
} packet;

void main() {
	int server_fd, client_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	int arr[SIZE];
	
	for(int i = 0; i < SIZE; i++)
		arr[i] = -1;
	
	printf("Go-Back-N ARQ\nTCP Server\n");
	
	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Socket creation failed!\n");
		exit(1);
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	
	if(bind(server_fd, (struct sockaddr*) &address, addrlen) < 0) {
		printf("Socket binding failed!\n");
		exit(1);
	}
	
	if(listen(server_fd, 5) < 0) {
		printf("Listening failed!\n");
		exit(1);
	}
	
	if((client_fd = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen)) < 0) {
		printf("Connection failed!\n");
		exit(1);
	} else {
		printf("Connected to client.\n");
	}
	
	packet p;
	int exp_seq = 0, flag = 0;
	
	while(1) {
		int status = recv(client_fd, &p, sizeof(packet), 0);
		
		if(status < 0) {
			printf("Receive failed!\n");
		} else if (status == 0) {
			printf("Receive completed.\nArray: ");
			
			for(int i = 0; arr[i] != -1; i++) {
				printf("%d ", arr[i]);
			}
			
			printf("\n");
			
			break;
		} else {
			if(p.seq > exp_seq) {
				if(!flag) {
					flag = 1;
					
					p.type = -1;
					
					p.seq = exp_seq;
					
					if(send(client_fd, &p, sizeof(packet), 0) < 0) {
						printf("Send failed!\n");
					} else {
						printf("Sent: NACK %d\n", p.seq);
					}
				}
				
				continue;
			} else {
				flag = 0;
				
				exp_seq = p.seq + 1;
			}
			
			p.type = 1;
			
			printf("Received: %d (SEQ %d)\n", p.data, p.seq);
			arr[p.seq] = p.data;
			
			if(rand() % 10 != 6) {
				if(send(client_fd, &p, sizeof(packet), 0) < 0) {
					printf("Send failed!\n");
				} else {
					printf("Sent: ACK %d\n", p.seq);
				}
			} else {
				printf("ACK %d lost\n", p.seq);
			}
		}
	}
	
	close(server_fd);
	close(client_fd);
}
