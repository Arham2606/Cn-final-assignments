#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>

int main() {

int port[] = {9000,9001,9002,9003,9004};
int hop[] = {3,1,0,4,2};

int receiverSocket[5];

struct sockaddr_in receiverAddress, transmitterAddress;

socklen_t len = sizeof(transmitterAddress);

char buffer[256];

int totalHop = sizeof(hop)/sizeof(hop[0]);

for(int i=0; i<5; i++) {

receiverSocket[i] = socket(AF_INET, SOCK_DGRAM, 0);

receiverAddress.sin_family = AF_INET;
receiverAddress.sin_port = htons(port[i]);
receiverAddress.sin_addr.s_addr = INADDR_ANY;

bind(receiverSocket[i], (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));

}

for(int i=0; i<7; i++) {

recvfrom(receiverSocket[hop[i%totalHop]], buffer, sizeof(buffer), 0, (struct sockaddr *)&transmitterAddress, &len);

printf("Received: %s\n", buffer);

}

for(int i=0; i<5; i++)
close(receiverSocket[i]);

return 0;
}
