#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 8000

typedef struct packet {
	int data;
	int type; // SEQ (0) or ACK (1)
	int seq; // Sequence number (0 or 1)
} packet;

typedef struct data {
	int* arr;
	int* i;
	int client_fd;
	packet* p;
} data;

void* client(void* arg) {
	data d = *((data*) arg);
	
	d.p->type = 0;
	d.p->data = d.arr[*d.i];
	
	if(rand() % 5 != 2) {
		if(send(d.client_fd, d.p, sizeof(packet), 0) < 0) {
			printf("Send failed!\n");
		} else {
			printf("Sent: %d (SEQ %d)\n", d.p->data, d.p->seq);
			
			if(recv(d.client_fd, d.p, sizeof(packet), 0) < 0) {
				printf("Receive failed!\n");
			} else {
				printf("Received: ACK %d\n", d.p->seq);
				
				d.arr[*d.i] = -1;
				
				*(d.i) = *(d.i) + 1;
			}
		}
	} else {
		printf("SEQ %d lost\n", d.p->seq);
	}
}

void* timeout(void* t) {
	sleep(1);
	pthread_t tid = *((pthread_t*) t);
	pthread_cancel(tid);
}

void main() {
	int client_fd;
	struct sockaddr_in serv_addr;
	
	printf("TCP Client\n");
	
	client_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(client_fd < 0) {
		printf("Socket creation failed!\n");
		exit(1);
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT);
	
	if(connect(client_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Connection failed!\n");
		exit(1);
	} else {
		printf("Connected to server.\n");
	}
	
	int n;
	
	printf("Enter array size: ");
	scanf("%d", &n);
	
	int arr[n];
	
	printf("Enter array elements: ");
	for(int i = 0; i < n; i++) {
		scanf("%d", &arr[i]);
	}
	
	int i = 0;
	packet p;
	data d;
	d.client_fd = client_fd;
	d.p = &p;
	d.arr = arr;
	d.i = &i;
	p.seq = 0;
	pthread_t tid1, tid2;
	
	while(1) {
		if(i == n) {
			printf("Send completed.\nArray: ");
			
			for(int j = 0; j < n; j++) {
				printf("%d ", arr[j]);
			}
			
			printf("\n");
			break;
		}
		pthread_create(&tid1, NULL, client, &d);
		pthread_create(&tid2, NULL, timeout, &tid1);
		pthread_join(tid1, NULL);
		pthread_join(tid2, NULL);
	}
	
	close(client_fd);
}
