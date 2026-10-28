#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    char flag[] = "01111110";
    char ESC[] = "00011011";
    char dataRecv[2000];
    char data[2000] = "";

    int receiverSocket;
    struct sockaddr_in receiverAddress, transmitterAddress;
    socklen_t len = sizeof(transmitterAddress);

    receiverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(9000);
    receiverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(receiverSocket, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));

    recvfrom(receiverSocket, dataRecv, sizeof(dataRecv) - 1, 0, (struct sockaddr *)&transmitterAddress, &len);

    int i = 8, k = 0;
    int count = 1;

    while (dataRecv[i] != '\0') {
        int matchESC = 1;
        int matchFlag = 1;

        for (int j = 0; j < 8; j++) {
            if (dataRecv[i + j] != flag[j]) {
                matchFlag = 0;
                break;
            }
        }

        if (matchFlag) {
            count++;
            if (count == 2) {
                data[k] = '\0';
                printf("Frame: %s\n", data);
                k = 0;
                count = 1;
            }
            i += 16;
            continue;
        }

        for (int j = 0; j < 8; j++) {
            if (dataRecv[i + j] != ESC[j]) {
                matchESC = 0;
                break;
            }
        }

        if (matchESC) {
            i += 8;
            for (int j = 0; j < 8; j++) data[k++] = dataRecv[i++];
        } else {
            data[k++] = dataRecv[i++];
        }
    }

    printf("\nReceived Data: %s\n\n", dataRecv);

    close(receiverSocket);
    return 0;
}
