#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8000

int count = 0;

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
	packet* p;
	window* w;
} data;

int ackFrame(int* arr, int index) {
	int flag = -1;
	
	for(int i = 0; i < index; i++) {
		if(arr[i] != -1) {
			flag = i;
			break;
		}
	}
	
	arr[index] = -1;
	
	return flag;
}

void sendWindow(data d) {
	for(d.p->seq = d.w->start; d.p->seq <= d.w->end && d.p->seq < d.n; d.p->seq++) {
		d.p->type = 0;
		d.p->data = d.arr[d.p->seq];
		
		if(d.p->data == -1)
			continue;
		
		if(rand() % 10 != 6) {
			if(send(d.client_fd, d.p, sizeof(packet), 0) < 0) {
				printf("Send failed!\n");
			} else {
				printf("Sent: %d (SEQ %d)\n", d.p->data, d.p->seq);
			}
		} else {
			printf("Lost: %d (SEQ %d)\n", d.p->data, d.p->seq);
		}
	}
}

void sendFrame(data d, int seq) {
	d.p->type = 0;
	int temp;
	
	if(seq == -1)
		d.p->data = d.arr[d.w->end];
	else {
		d.p->data = d.arr[seq];
		temp = d.p->seq;
		d.p->seq = seq;
	}
	
	if(d.p->data == -1)
		return;
	
	if(rand() % 10 != 6) {
		if(send(d.client_fd, d.p, sizeof(packet), 0) < 0) {
			printf("Send failed!\n");
		} else {
			printf("Sent: %d (SEQ %d)\n", d.p->data, d.p->seq);
		}
	} else {
		printf("Lost: %d (SEQ %d)\n", d.p->data, d.p->seq);
	}
	
	if(seq == -1)
		d.p->seq = d.p->seq + 1;
	else
		d.p->seq = temp;
}

void recvAck(data d) {
	data d1;
	packet p;
	d1.p = &p;
	
	if(recv(d.client_fd, d1.p, sizeof(packet), 0) < 0) {
		printf("Time out! Window retransmitting.\n");
		sendWindow(d);
		recvAck(d);
	} else {
		if(d1.p->type == 1) {
			if(d.arr[d1.p->seq] == -1) {
				recvAck(d);
			} else {
				printf("Received: ACK %d\n", d1.p->seq);
				
				count++;
				
				d.w->start++;
				d.w->end++;
				
				int index = ackFrame(d.arr, d1.p->seq);
				
				if(index != -1) {
					printf("ACK %d not received! Frame %d retransmitting.\n", index, index);
					
					sendFrame(d, index);
				}
				
				if(count == d.n) {
					printf("Send completed.\nArray: ");
					
					for(int i = 0; i < d.n; i++) {
						printf("%d ", d.arr[i]);
					}
					
					printf("\n");
					
					close(d.client_fd);
					exit(0);
				}
				
				if(d.w->end < d.n) {
					sendFrame(d, -1);
		
					recvAck(d);
				}
				else
					recvAck(d);
			}
		} else if(d1.p->type == -1) {
			printf("Received: NACK %d. Frame %d retransmitting.\n", d1.p->seq, d1.p->seq);
			
			sendFrame(d, d1.p->seq);
			
			recvAck(d);
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
	p.seq = 0;
	
	sendWindow(d);
	recvAck(d);
}
