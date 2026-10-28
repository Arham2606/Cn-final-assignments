#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    char data[100] = "";
    char recv[100];
    int receiverSocket;
    struct sockaddr_in receiverAddress, transmitterAddress;
    socklen_t len = sizeof(transmitterAddress);
    int receivedData[400];

    receiverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(9000);
    receiverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(receiverSocket, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));

    recvfrom(receiverSocket, recv, sizeof(recv) - 1, 0, (struct sockaddr *)&transmitterAddress, &len);

    printf("\nReceived Data: %s\n", recv);

    int i = 0;
    while (recv[i] != '\0') {
        char frame[20] = "";
        int k = 0;
        int len = (recv[i] - '0') - 1;

        for (int j = i + 1; j < 1 + len + i; j++) frame[k++] = recv[j];

        i += len + 1;
        frame[k] = '\0';
        printf("\nFrame: %s", frame);
    }

    printf("\n");

    close(receiverSocket);
    return 0;
}
