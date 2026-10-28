#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

int main() {
int clientsocket;

struct sockaddr_in serveraddress;

clientsocket = socket(AF_INET, SOCK_DGRAM, 0);
if(clientsocket<0)
printf("socket is not created\n");
else 
printf("socket is created succesfully\n");

serveraddress.sin_family=AF_INET;
serveraddress.sin_port=htons(9000);
serveraddress.sin_addr.s_addr= INADDR_ANY;

char clientmessage[256] = "Hello server";

sendto(clientsocket, clientmessage, sizeof(clientmessage), 0, (struct sockaddr *)&serveraddress, sizeof(serveraddress));

char serverresponse[256];
int len = sizeof(serveraddress);

recvfrom(clientsocket, serverresponse, sizeof(serverresponse), 0, (struct sockaddr *)&serveraddress, &len);

printf("The reply from the server is: %s\n", serverresponse);

close(clientsocket);

return 0;
}
