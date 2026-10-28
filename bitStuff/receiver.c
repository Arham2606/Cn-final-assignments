#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>

int main() {
    char flag[] = "01111110";
    char received[400];
    char data[400] = "";

    int receiverSocket;

    struct sockaddr_in receiverAddress, transmitterAddress;

    socklen_t len = sizeof(transmitterAddress);

    receiverSocket = socket(AF_INET,SOCK_DGRAM,0);

    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(9000);
    receiverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(receiverSocket, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));

    recvfrom(receiverSocket, received, sizeof(received)-1, 0, (struct sockaddr *)&transmitterAddress, &len);
    
    int i = 8, k = 0, count  = 0;
    
    while(received[i] != '\0') {
        int match = 1;
        
        for(int j=0; j<8; j++) {
            if(received[i+j] != flag[j]) {
                match = 0;
                break;
            }
        }
        
        if(match) {
            break;
        }
        
        data[k++] = received[i];
        
        if(received[i] == '1') {
            count++;
        }
        else {
            count = 0;
        }
        
        if(count == 5) {
            i++;
            count = 0;
        }
        
        i++;
    }
    
    data[k] = '\0';
    
    printf("Received data: %s\n\n", received);
    
    printf("Original data: %s\n", data);

    printf("\n");

    close(receiverSocket);

    return 0;
}
