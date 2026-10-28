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
    char data[10][200];
    char newData[200] = "";
    int n;

    printf("Enter no. of frames: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        printf("Enter data: ");
        scanf("%s", data[i]);
    }

    int k = 0;

    for (int l = 0; l < n; l++) {
        for (int j = 0; j < 8; j++) newData[k++] = flag[j];

        int i = 0;
        while (data[l][i] != '\0') {
            int matchFlag = 1;
            int matchESC = 1;

            for (int j = 0; j < 8; j++) {
                if (data[l][i + j] != flag[j]) {
                    matchFlag = 0;
                    break;
                }
            }

            for (int j = 0; j < 8; j++) {
                if (data[l][i + j] != ESC[j]) {
                    matchESC = 0;
                    break;
                }
            }

            if (matchFlag || matchESC) {
                for (int j = 0; j < 8; j++) newData[k++] = ESC[j];
                for (int j = 0; j < 8; j++) newData[k++] = data[l][i++];
            } else {
                newData[k++] = data[l][i++];
            }
        }

        for (int j = 0; j < 8; j++) newData[k++] = flag[j];
    }

    newData[k] = '\0';

    int transmitterSocket = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in receiverAddress;
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(9000);
    receiverAddress.sin_addr.s_addr = INADDR_ANY;

    sendto(transmitterSocket, newData, k, 0, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));

    printf("\nUser Data: \n");
    for (int i = 0; i < n; i++) printf("Frame: %s\n", data[i]);

    printf("\nStuffed Data : %s\n\n", newData);

    close(transmitterSocket);
    return 0;
}
