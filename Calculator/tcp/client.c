#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);

    if (clientfd == -1) {
        perror("socket");
        return 1;
    }

    printf("Client socket created successfully\n");

    struct sockaddr_in serverAddress = {0};

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5000);
    serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(clientfd, (struct sockaddr *)&serverAddress,
                sizeof(serverAddress)) == -1) {
        perror("connect");
        close(clientfd);
        return 1;
    }

    printf("Connected to server successfully\n");

    double num1, num2;
    char operator;

    printf("Enter first number: ");
    scanf("%lf", &num1);

    printf("Enter operator (+, -, *, /): ");
    scanf(" %c", &operator);

    printf("Enter second number: ");
    scanf("%lf", &num2);

    char request[1024];

    snprintf(request, sizeof(request), "%.15g,%.15g,%c",
             num1, num2, operator);

    if (send(clientfd, request, strlen(request), 0) == -1) {
        perror("send");
        close(clientfd);
        return 1;
    }

    printf("Request sent to server\n");

    char buffer[1024];

    ssize_t received = recv(clientfd, buffer, sizeof(buffer) - 1, 0);

    if (received <= 0) {
        if (received == -1)
            perror("recv");
        else
            printf("Server disconnected without sending a response\n");

        close(clientfd);
        return 1;
    }

    buffer[received] = '\0';

    printf("Result from server: %s\n", buffer);

    close(clientfd);

    return 0;
}