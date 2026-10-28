#include<stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

int main() {
int serversocket, bindstatus;
char servermessage[256] = "Hello Client and we are connected now";
char buffer[256];

struct sockaddr_in serveraddress, clientaddress;
socklen_t len = sizeof(clientaddress);

serversocket = socket(AF_INET, SOCK_DGRAM, 0);
if(serversocket < 0) 
printf("Failed");

serveraddress.sin_family = AF_INET;
serveraddress.sin_port = htons(9000);
serveraddress.sin_addr.s_addr = INADDR_ANY;

bindstatus = bind(serversocket, (struct sockaddr *)&serveraddress, sizeof(serveraddress));
if(bindstatus < 0)
printf("binding failed\n");-
else
printf("binding successful\n");

recvfrom(serversocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientaddress, &len);

printf("Client Message: %s\n", buffer);

sendto(serversocket, servermessage, sizeof(servermessage), 0, (struct sockaddr *)&clientaddress, len);

close(serversocket);

return 0;
}
