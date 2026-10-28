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

char *reply[] = {"Hello",", ","My ","name ","is ","Om ","Reddy"};

int transmitterSocket[5];

struct sockaddr_in receiverAddress[5];

int totalHop = sizeof(hop)/sizeof(hop[0]);
int totalMsg = sizeof(reply)/sizeof(reply[0]);

for(int i=0; i<5; i++) {

transmitterSocket[i] = socket(AF_INET,SOCK_DGRAM,0);

receiverAddress[i].sin_family = AF_INET;
receiverAddress[i].sin_port = htons(port[i]);
receiverAddress[i].sin_addr.s_addr = INADDR_ANY;

}

for(int i=0; i<totalMsg; i++) {

sendto(transmitterSocket[hop[i%totalHop]], reply[i], strlen(reply[i])+1, 0, (struct sockaddr *)&receiverAddress[hop[i%totalHop]], sizeof(receiverAddress[hop[i%totalHop]]));

printf("Sent: %s\n", reply[i]);

sleep(1);

}

for(int i=0; i<5; i++)
close(transmitterSocket[i]);

return 0;
}
