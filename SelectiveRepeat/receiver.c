#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

typedef struct BufferNode
{
    int packet;
    struct BufferNode *next;
} BufferNode;


/* Add packet to receiver buffer */
void addBuffer(BufferNode **head,int packet)
{
    BufferNode *newnode =
        (BufferNode *)malloc(sizeof(BufferNode));

    newnode->packet = packet;
    newnode->next = NULL;

    if(*head == NULL)
    {
        *head = newnode;
        return;
    }

    BufferNode *temp = *head;

    while(temp->next != NULL)
        temp = temp->next;

    temp->next = newnode;
}


/* Check whether packet is already in buffer */
int inBuffer(BufferNode *head,int packet)
{
    BufferNode *temp = head;

    while(temp != NULL)
    {
        if(temp->packet == packet)
            return 1;

        temp = temp->next;
    }

    return 0;
}


/* Remove packet from buffer */
void removeBuffer(BufferNode **head,int packet)
{
    BufferNode *temp = *head;
    BufferNode *prev = NULL;

    while(temp != NULL)
    {
        if(temp->packet == packet)
        {
            if(prev == NULL)
                *head = temp->next;
            else
                prev->next = temp->next;

            free(temp);
            return;
        }

        prev = temp;
        temp = temp->next;
    }
}


/* Print receiver buffer */
void printBuffer(BufferNode *head)
{
    BufferNode *temp = head;

    printf("Receiver Buffer: ");

    if(temp == NULL)
    {
        printf("EMPTY\n");
        return;
    }

    while(temp != NULL)
    {
        printf("[%d] ",temp->packet);
        temp = temp->next;
    }

    printf("\n");
}


/* Check whether packet is available in buffer */
int bufferContains(BufferNode *head,int packet)
{
    return inBuffer(head,packet);
}


int main()
{
    int sock;

    struct sockaddr_in receiver,transmitter;

    int packet;

    int total_packets;
    int window_size;

    int tt,tp,loss;

    int expected = 1;
    int counter = 0;

    /*
       Receiver buffer
    */

    BufferNode *buffer = NULL;

    /*
       Remember packets that were
       already simulated as lost.
    */

    int lost[100000];

    for(int i=0;i<100000;i++)
        lost[i] = 0;

    /*
       SOCKET
    */

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr = INADDR_ANY;

    sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    bind(sock,
         (struct sockaddr *)&receiver,
         sizeof(receiver));

    socklen_t len =
        sizeof(transmitter);

    /*
       RECEIVE PARAMETERS
    */

    recvfrom(sock,
             &total_packets,
             sizeof(total_packets),
             0,
             (struct sockaddr *)&transmitter,
             &len);

    recvfrom(sock,
             &window_size,
             sizeof(window_size),
             0,
             (struct sockaddr *)&transmitter,
             &len);

    recvfrom(sock,
             &tt,
             sizeof(tt),
             0,
             (struct sockaddr *)&transmitter,
             &len);

    recvfrom(sock,
             &tp,
             sizeof(tp),
             0,
             (struct sockaddr *)&transmitter,
             &len);

    recvfrom(sock,
             &loss,
             sizeof(loss),
             0,
             (struct sockaddr *)&transmitter,
             &len);

    /*
       START RECEIVER
    */

    printf("Receiver Started (Selective Repeat)\n");

    printf("Expecting %d packets, window size %d, loss interval = %d\n\n",
           total_packets,
           window_size,
           loss);

    /*
       MAIN LOOP
    */

    while(expected <= total_packets)
    {
        recvfrom(sock,
                 &packet,
                 sizeof(packet),
                 0,
                 (struct sockaddr *)&transmitter,
                 &len);

        /*
           =================================================
           CASE 1: EXPECTED PACKET
           =================================================
        */

        if(packet == expected)
        {
            /*
               Packet has not been lost before
            */

            if(lost[packet] == 0)
            {
                counter++;

                /*
                   SIMULATE LOSS
                */

                if(counter == loss)
                {
                    lost[packet] = 1;

                    printf("Packet %d LOST (simulated, position %d in streak)\n",
                           packet,
                           counter);

                    printf("Receiver Buffer: ");
                    printBuffer(buffer);

                    printf("\n");

                    /*
                       No ACK.
                       expected does not move.
                    */

                    continue;
                }

                /*
                   NORMAL EXPECTED PACKET
                */

                printf("Packet %d received\n",
                       packet);

                /*
                   ACK INDIVIDUALLY
                */

                sendto(sock,
                       &packet,
                       sizeof(packet),
                       0,
                       (struct sockaddr *)&transmitter,
                       len);

                printf("ACK %d sent (individual)\n",
                       packet);

                /*
                   Deliver immediately
                */

                printf("Packet %d delivered to upper layer\n",
                       packet);

                expected++;

                printf("Receiver Buffer: ");
                printBuffer(buffer);

                printf("\n");
            }

            /*
               Packet was previously lost
               and is now recovered.
            */

            else
            {
                printf("Packet %d received (recovered after earlier loss)\n",
                       packet);

                /*
                   ACK recovered packet
                */

                sendto(sock,
                       &packet,
                       sizeof(packet),
                       0,
                       (struct sockaddr *)&transmitter,
                       len);

                printf("ACK %d sent (individual)\n",
                       packet);

                /*
                   Deliver packet
                */

                printf("Packet %d delivered to upper layer\n",
                       packet);

                expected++;

                /*
                   The lost packet has been recovered,
                   so restart loss counter.
                */

                counter = 1;

                /*
                   Now check whether subsequent packets
                   are already present in receiver buffer.
                */

                while(bufferContains(buffer,expected))
                {
                    printf("Packet %d removed from receiver buffer\n",
                           expected);

                    removeBuffer(&buffer,expected);

                    printf("Packet %d delivered to upper layer\n",
                           expected);

                    expected++;
                }

                printf("Receiver Buffer: ");
                printBuffer(buffer);

                printf("\n");
            }
        }

        /*
           =================================================
           CASE 2: OUT-OF-ORDER PACKET
           =================================================
        */

        else if(packet > expected &&
                packet < expected + window_size)
        {
            /*
               Packet is inside the SR receiver window.
               Therefore ACCEPT and BUFFER it.
            */

            if(!inBuffer(buffer,packet))
            {
                printf("Packet %d received (out of order)\n",
                       packet);

                addBuffer(&buffer,packet);

                printf("Packet %d added to receiver buffer\n",
                       packet);

                /*
                   Individual ACK
                */

                sendto(sock,
                       &packet,
                       sizeof(packet),
                       0,
                       (struct sockaddr *)&transmitter,
                       len);

                printf("ACK %d sent (individual)\n",
                       packet);

                printBuffer(buffer);

                printf("\n");
            }
            else
            {
                /*
                   Duplicate packet
                */

                printf("Duplicate packet %d received\n",
                       packet);

                sendto(sock,
                       &packet,
                       sizeof(packet),
                       0,
                       (struct sockaddr *)&transmitter,
                       len);

                printf("ACK %d resent (individual)\n",
                       packet);

                printBuffer(buffer);

                printf("\n");
            }
        }

        /*
           =================================================
           CASE 3: OLD / OUT-OF-WINDOW PACKET
           =================================================
        */

        else
        {
            printf("Packet %d discarded (outside receiver window, expected %d)\n",
                   packet,
                   expected);

            printf("Receiver Buffer: ");
            printBuffer(buffer);

            printf("\n");
        }
    }

    printf("All packets received successfully.\n");

    close(sock);

    return 0;
}