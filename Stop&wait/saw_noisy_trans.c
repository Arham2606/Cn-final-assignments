#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void)
{
    int sock;
    int size = 0, tt = 0, tp = 0, loss = 2;
    int total_time = 0;

    struct sockaddr_in receiver, from;
    socklen_t len = sizeof(from);

    printf("Enter number of packets to transmit: ");
    scanf("%d", &size);

    printf("Enter Transmission Time: ");
    scanf("%d", &tt);

    printf("Enter Propagation Time: ");
    scanf("%d", &tp);

    printf("Enter loss factor (every Nth packet is lost once): ");
    scanf("%d", &loss);

    if (loss < 2)
        loss = 2;

    /*
     * RTT used for Stop-and-Wait:
     *
     * RTT = 2 * Transmission Time + 2 * Propagation Time
     */
    int rtt = 2 * tt + 2 * tp;

    /*
     * Three timer values given by ma'am:
     *
     * T0 = 1 RTT
     * T1 = 3 RTT
     * T2 = 4 RTT
     */
    int t0 = 1 * rtt;
    int t1 = 3 * rtt;
    int t2 = 4 * rtt;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    /*
     * Timeout is T1 = 3 RTT.
     */
    struct timeval tv;

    tv.tv_sec = t1;
    tv.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               &tv, sizeof(tv));

    memset(&receiver, 0, sizeof(receiver));

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = inet_addr("127.0.0.1");

    /*
     * Send setup information.
     */
    sendto(sock, &size, sizeof(size), 0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    sendto(sock, &tt, sizeof(tt), 0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    sendto(sock, &tp, sizeof(tp), 0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    sendto(sock, &loss, sizeof(loss), 0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    int frame = 1;

    while (frame <= size)
    {
        int ack = -1;

        printf("\n----------------------------------------\n");
        printf("Packet %d\n", frame);

        /*
         * Send packet.
         */
        printf("Packet %d sent\n", frame);

        sendto(sock, &frame, sizeof(frame), 0,
               (struct sockaddr *)&receiver,
               sizeof(receiver));

        /*
         * T0 = 1 RTT
         *
         * ACK should ideally arrive here.
         */
        printf("T0 = 1 RTT  -> %d\n", t0);

        if (recvfrom(sock, &ack, sizeof(ack), 0,
                     (struct sockaddr *)&from, &len) >= 0)
        {
            if (ack == frame)
            {
                printf("ACK %d received at T0 = %d\n",
                       ack, t0);

                total_time += t0;

                printf("Packet %d completed successfully\n",
                       frame);

                printf("Total time = %d\n",
                       total_time);

                frame++;
            }
        }
        else
        {
            /*
             * No ACK by T0.
             *
             * Wait until T1 = 3 RTT.
             *
             * Since socket timeout itself is T1,
             * this point represents timeout.
             */
            printf("T0 expired - ACK not received\n");

            printf("T1 = 3 RTT -> %d\n", t1);

            printf("TIMEOUT for packet %d\n", frame);

            /*
             * Retransmit packet.
             */
            printf("Retransmitting packet %d\n", frame);

            sendto(sock, &frame, sizeof(frame), 0,
                   (struct sockaddr *)&receiver,
                   sizeof(receiver));

            /*
             * T2 = 4 RTT
             *
             * Retransmitted packet + ACK.
             */
            printf("T2 = 4 RTT -> %d\n", t2);

            /*
             * Wait for ACK of retransmitted packet.
             */
            if (recvfrom(sock, &ack, sizeof(ack), 0,
                         (struct sockaddr *)&from, &len) >= 0)
            {
                if (ack == frame)
                {
                    printf("ACK %d received at T2 = %d\n",
                           ack, t2);

                    total_time += t2;

                    printf("Packet %d recovered successfully\n",
                           frame);

                    printf("Total time = %d\n",
                           total_time);

                    frame++;
                }
            }
        }

        fflush(stdout);
    }

    printf("\n========================================\n");
    printf("All %d packets transmitted successfully.\n",
           size);

    printf("Total time required: %d\n", total_time);

    printf("========================================\n");

    close(sock);

    return 0;
}