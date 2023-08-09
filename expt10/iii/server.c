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

int add(int* arr, int key, int index) {
	int flag = -1;
	
	for(int i = 0; i < index; i++) {
		if(arr[i] == -1) {
			flag = i;
			break;
		}
	}
	
	arr[index] = key;
	
	return flag;
}

void main() {
	int server_fd, client_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	
	printf("Selective Repeat ARQ\nTCP Server\n");
	
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
	
	int* arr = malloc(SIZE * sizeof(int));
	
	for(int i = 0; i < SIZE; i++)
		arr[i] = -1;
	
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
			printf("Received: %d (SEQ %d)\n", p.data, p.seq);
			
			int index = add(arr, p.data, p.seq);
			
			if(index != -1) {
				int temp = p.seq;
				
				p.type = -1;
				
				p.seq = index;
				
				if(rand() % 10 != 6) {
					if(send(client_fd, &p, sizeof(packet), 0) < 0) {
						printf("Send failed!\n");
					} else {
						printf("Sent: NACK %d\n", p.seq);
					}
				} else {
					printf("Lost: NACK %d\n", p.seq);
				}
				
				p.seq = temp;
			}
			
			p.type = 1;
			
			if(rand() % 10 != 6) {
				if(send(client_fd, &p, sizeof(packet), 0) < 0) {
					printf("Send failed!\n");
				} else {
					printf("Sent: ACK %d\n", p.seq);
				}
			} else {
				printf("Lost: ACK %d\n", p.seq);
			}
		}
	}
	
	close(server_fd);
	close(client_fd);
}
