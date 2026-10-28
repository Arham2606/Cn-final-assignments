#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int len(char len[10]) {
    int n = 0;
    while (len[n] != '\0') n++;
    return n + 1;
}

int main() {
    char data[100][10];
    char res[100] = "";
    int n;

    printf("Enter no. of frames: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++) scanf("%s", data[i]);

    int k = 0;
    for (int i = 0; i < n; i++) {
        int length = len(data[i]);
        res[k++] = length + '0';
        for (int j = 0; j < length - 1; j++) res[k++] = data[i][j];
    }
    res[k] = '\0';

    int transmitterSocket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in receiverAddress;
    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(9000);
    receiverAddress.sin_addr.s_addr = INADDR_ANY;

    sendto(transmitterSocket, res, k, 0, (struct sockaddr *)&receiverAddress, sizeof(receiverAddress));

    printf("\nOriginal Data : ");
    for (int i = 0; i < n; i++) printf("%s", data[i]);
    printf("\nData sent : %s\n\n", res);

    close(transmitterSocket);
    return 0;
}
