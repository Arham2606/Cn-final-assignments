#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>

int main() {
    char flag[] = "01111110";
    char data[400];
    char stuffed[500] = "";
    
    printf("Enter data: ");
    scanf("%s", data);
    
    int i = 0, k = 0, count = 0;
    
    for(int j=0; j<8; j++) {
        stuffed[k++] = flag[j];
    }
    
    while(data[i] != '\0') {
        
        stuffed[k++] = data[i];
        
        if(data[i] == '1') {
            count++;
        }
        else {
            count = 0;
        }
        
        if(count == 5) {
            stuffed[k++] = '0';
            count = 0;
        }
        
        i++;
    }
    
    for(int j=0; j<8; j++) {
        stuffed[k++] = flag[j];
    }
    
    stuffed[k] = '\0';

    int transmitterSocket;

    struct sockaddr_in receiverAddress;

    transmitterSocket = socket(AF_INET,SOCK_DGRAM,0);

    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(9000);
    receiverAddress.sin_addr.s_addr = INADDR_ANY;

    sendto(transmitterSocket, stuffed, k, 0, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));
    
    printf("\nStuffed Data : %s\n\n", stuffed);

    close(transmitterSocket);

    return 0;
}
