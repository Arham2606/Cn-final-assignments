#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
    int sock, size = 0, tt = 0, tp = 0, loss = 2;
    struct sockaddr_in sender, receiver;
    socklen_t len = sizeof(sender);

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&receiver, 0, sizeof(receiver));

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&receiver, sizeof(receiver)) < 0)
    {
        perror("bind");
        exit(1);
    }

    recvfrom(sock, &size, sizeof(size), 0,
             (struct sockaddr *)&sender, &len);

    recvfrom(sock, &tt, sizeof(tt), 0,
             (struct sockaddr *)&sender, &len);

    recvfrom(sock, &tp, sizeof(tp), 0,
             (struct sockaddr *)&sender, &len);

    recvfrom(sock, &loss, sizeof(loss), 0,
             (struct sockaddr *)&sender, &len);

    printf("\n========== STOP AND WAIT RECEIVER ==========\n");
    printf("Expecting %d packets\n", size);
    printf("Transmission Time = %d\n", tt);
    printf("Propagation Time  = %d\n", tp);
    printf("Loss factor       = %d\n\n", loss);

    int expected = 1;
    int lastDropped = 0;

    while (expected <= size)
    {
        int frame = 0;

        recvfrom(sock, &frame, sizeof(frame), 0,
                 (struct sockaddr *)&sender, &len);

        /*
         * Simulate loss only once for every Nth packet.
         */
        if (frame % loss == 0 && frame != lastDropped)
        {
            lastDropped = frame;

            printf("Packet %d LOST on channel\n", frame);
            printf("No ACK sent for packet %d\n\n", frame);

            fflush(stdout);
            continue;
        }

        if (frame == expected)
        {
            printf("Packet %d received successfully\n", frame);
            expected++;
        }
        else
        {
            printf("Duplicate packet %d received\n", frame);
            printf("Re-acknowledging packet %d\n", frame);
        }

        sendto(sock, &frame, sizeof(frame), 0,
               (struct sockaddr *)&sender, len);

        printf("ACK %d sent\n\n", frame);

        fflush(stdout);
    }

    printf("All %d packets received successfully.\n", size);

    close(sock);
    return 0;
}