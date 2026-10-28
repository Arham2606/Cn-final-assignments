#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 9000
#define PN_SEQUENCE "10110"

// Manual XOR
int manual_xor(int a, int b)
{
    if (a == 0 && b == 0)
        return 0;
    else if (a == 0 && b == 1)
        return 1;
    else if (a == 1 && b == 0)
        return 1;
    else
        return 0;
}

// Spread the message
void spread(const char *msg, char *spread_sequence)
{
    int msg_length = strlen(msg);
    int PN_length = strlen(PN_SEQUENCE);
    int seq_index = 0;

    for (int i = 0; i < msg_length; i++)
    {
        int data_bit = msg[i] - '0';

        for (int j = 0; j < PN_length; j++)
        {
            int pn_chip = PN_SEQUENCE[j] - '0';

            int spread_chip =
                manual_xor(data_bit, pn_chip);

            spread_sequence[seq_index++] =
                '0' + spread_chip;
        }
    }

    spread_sequence[seq_index] = '\0';
}

int main()
{
    char msg[100];

    printf("Enter binary message: ");
    scanf("%99s", msg);

    int msg_length = strlen(msg);
    int PN_length = strlen(PN_SEQUENCE);

    // Validate binary message
    for (int i = 0; i < msg_length; i++)
    {
        if (msg[i] != '0' && msg[i] != '1')
        {
            printf("Error: Message must contain only 0 and 1.\n");
            return 1;
        }
    }

    char spread_sequence[msg_length * PN_length + 1];

    // Create UDP client socket
    int clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (clientSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Configure server address
    struct sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    // Spread the message
    spread(msg, spread_sequence);

    printf("Original message : %s\n", msg);
    printf("PN sequence      : %s\n", PN_SEQUENCE);
    printf("Spread sequence  : %s\n", spread_sequence);

    // Send entire spread sequence in one UDP datagram
    sendto(clientSocket, spread_sequence,
           strlen(spread_sequence), 0,
           (struct sockaddr *)&serverAddress,
           sizeof(serverAddress));

    printf("Spread message sent successfully.\n");

    close(clientSocket);

    return 0;
}
