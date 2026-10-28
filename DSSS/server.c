```c
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

// Despread the received sequence
void despread(const char *received_sequence,
              int num_blocks, char *decoded_msg)
{
    int PN_length = strlen(PN_SEQUENCE);

    for (int block = 0; block < num_blocks; block++)
    {
        int cnt_0 = 0;
        int cnt_1 = 0;

        for (int chip = 0; chip < PN_length; chip++)
        {
            int index = block * PN_length + chip;

            int received_chip =
                received_sequence[index] - '0';

            int pn_chip = PN_SEQUENCE[chip] - '0';

            int recovered_chip =
                manual_xor(received_chip, pn_chip);

            if (recovered_chip == 0)
                cnt_0++;
            else
                cnt_1++;
        }

        if (cnt_1 == PN_length)
            decoded_msg[block] = '1';
        else if (cnt_0 == PN_length)
            decoded_msg[block] = '0';
        else
        {
            printf("Warning: Block %d chips disagreed. Using vote.\n",
                   block);

            decoded_msg[block] =
                (cnt_1 > cnt_0) ? '1' : '0';
        }
    }

    decoded_msg[num_blocks] = '\0';
}

int main()
{
    // Create UDP receiver socket
    int receiverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (receiverSocket < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Configure receiver address
    struct sockaddr_in receiverAddress;

    receiverAddress.sin_family = AF_INET;
    receiverAddress.sin_port = htons(PORT);
    receiverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind receiver socket
    if (bind(receiverSocket,
             (struct sockaddr *)&receiverAddress,
             sizeof(receiverAddress)) < 0)
    {
        perror("Bind failed");
        close(receiverSocket);
        return 1;
    }

   char received_sequence[1024];

    // Receive entire spread sequence in one datagram
    int received = recvfrom(receiverSocket,
                            received_sequence,
                            sizeof(received_sequence) - 1,
                            0, NULL, NULL);

    if (received < 0)
    {
        perror("Receive failed");
        close(receiverSocket);
        return 1;
    }

    received_sequence[received] = '\0';

    printf("Received spread sequence: %s\n",
           received_sequence);

    int total_chips = received;
    int PN_length = strlen(PN_SEQUENCE);

    // Validate received length
    if (total_chips == 0 || total_chips % PN_length != 0)
    {
        printf("Error: Invalid spread sequence length.\n");
        close(receiverSocket);
        return 1;
    }

    // Validate received binary data
    for (int i = 0; i < total_chips; i++)
    {
        if (received_sequence[i] != '0' &&
            received_sequence[i] != '1')
        {
            printf("Error: Received data is not binary.\n");
            close(receiverSocket);
            return 1;
        }
    }

    int num_blocks = total_chips / PN_length;

    char decoded_msg[num_blocks + 1];

    printf("PN sequence           : %s\n", PN_SEQUENCE);
    printf("Number of chips       : %d\n", total_chips);
    printf("Number of data blocks : %d\n", num_blocks);

    // Despread and decode
    despread(received_sequence, num_blocks, decoded_msg);

    printf("Decoded message       : %s\n", decoded_msg);

    close(receiverSocket);

    return 0;
}
```