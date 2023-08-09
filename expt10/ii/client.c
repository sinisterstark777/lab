#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8000

typedef struct packet {
	int data;
	int type; // SEQ (0), ACK (1) or NACK(-1)
	int seq; // Sequence number
} packet;

typedef struct window {
	int size;
	int start;
	int end;
} window;

typedef struct data {
	int* arr;
	int n;
	int client_fd;
	int exp_seq;
	packet* p;
	window* w;
} data;

void recvAck(data d);

void sendWindow(data d) {
	d.p->seq = d.w->start;
	
	for(int i = d.w->start; i <= d.w->end && i < d.n; i++) {
		d.p->type = 0;
		d.p->data = d.arr[i];
		
		if(rand() % 10 != 6) {
			if(send(d.client_fd, d.p, sizeof(packet), 0) < 0) {
				printf("Send failed!\n");
			} else {
				printf("Sent: %d (SEQ %d)\n", d.p->data, d.p->seq);
			}
		} else {
			printf("%d (SEQ %d) lost\n", d.p->data, d.p->seq);
		}
		
		d.p->seq = d.p->seq + 1;
	}
	
	recvAck(d);
}

void sendFrame(data d) {
	d.p->type = 0;
	d.p->data = d.arr[d.w->end];
	
	if(rand() % 10 != 6) {
		if(send(d.client_fd, d.p, sizeof(packet), 0) < 0) {
			printf("Send failed!\n");
		} else {
			printf("Sent: %d (SEQ %d)\n", d.p->data, d.p->seq);
		}
	} else {
		printf("%d (SEQ %d) lost\n", d.p->data, d.p->seq);
	}
	
	d.p->seq = d.p->seq + 1;
	
	recvAck(d);
}

void recvAck(data d) {
	data d1;
	packet p;
	d1.p = &p;
	
	if(recv(d.client_fd, d1.p, sizeof(packet), 0) < 0) {
		printf("Time out! Window retransmitting.\n");
		sendWindow(d);
	} else {
		if(d1.p->seq > d.exp_seq) {
			printf("ACK %d not received! Window retransmitting.\n", d.exp_seq);
			
			while(recv(d.client_fd, d1.p, sizeof(packet), 0) > 0);
			
			sendWindow(d);
			
			return;
		}
		
		if(d1.p->type == 1) {
			printf("Received: ACK %d\n", d1.p->seq);
			
			d.arr[d1.p->seq] = -1;
			
			d.w->start++;
			
			if(d.w->start == d.n) {
				printf("Send completed.\nArray: ");
				
				for(int i = 0; i < d.n; i++) {
					printf("%d ", d.arr[i]);
				}
				
				printf("\n");
				
				close(d.client_fd);
				exit(0);
			}
			
			d.w->end++;
			
			d.exp_seq = d1.p->seq + 1;
			
			if(d.w->end < d.n)
				sendFrame(d);
			else
				recvAck(d);
		} else if(d1.p->type == -1) {
			printf("Received: NACK %d. Window retransmitting.\n", d1.p->seq);
			sendWindow(d);
		}
	}
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
	
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	
	int n;
	window w;
	
	printf("Enter window size: ");
	scanf("%d", &w.size);
	
	w.start = 0;
	w.end = w.size - 1;

	printf("Enter array size: ");
	scanf("%d", &n);
	
	int arr[n];
	
	printf("Enter array elements: ");
	for(int i = 0; i < n; i++) {
		scanf("%d", &arr[i]);
	}
	
	packet p;
	data d;
	d.client_fd = client_fd;
	d.p = &p;
	d.w = &w;
	d.n = n;
	d.arr = arr;
	d.exp_seq = 0;
	p.seq = 0;
	
	sendWindow(d);
}
