#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/time.h>
#include<math.h>

typedef struct Node
{
    int data;
    int cycle;
    int relseq;
    int acked;
    struct Node *next;
} node;

typedef struct Queue
{
    node *front;
    node *rear;
} queue;

int isEmpty(queue *q)
{
    return q->rear == NULL;
}

queue *createQueue()
{
    queue *q = (queue *)malloc(sizeof(queue));

    q->front = NULL;
    q->rear = NULL;

    return q;
}

node *createNode(int data,int cycle,int relseq)
{
    node *newnode = (node *)malloc(sizeof(node));

    newnode->data = data;
    newnode->cycle = cycle;
    newnode->relseq = relseq;
    newnode->acked = 0;
    newnode->next = NULL;

    return newnode;
}

void Enqueue(queue *q,int data,int cycle,int relseq)
{
    node *newnode = createNode(data,cycle,relseq);

    if(isEmpty(q))
    {
        q->front = newnode;
        q->rear = newnode;
    }
    else
    {
        q->rear->next = newnode;
        q->rear = newnode;
    }
}

void printQueue(queue *q)
{
    node *temp = q->front;

    printf("Window: ");

    while(temp != NULL)
    {
        printf("%d(C%d:%d) ",
               temp->data,
               temp->cycle,
               temp->relseq);

        temp = temp->next;
    }

    printf("\n");
}

/* Remove only the packet which has been ACKed */
void removePacket(queue *q,int packet)
{
    node *temp = q->front;
    node *prev = NULL;

    while(temp != NULL)
    {
        if(temp->data == packet)
        {
            if(prev == NULL)
                q->front = temp->next;
            else
                prev->next = temp->next;

            if(temp == q->rear)
                q->rear = prev;

            free(temp);
            return;
        }

        prev = temp;
        temp = temp->next;
    }
}

node *findPacket(queue *q,int packet)
{
    node *temp = q->front;

    while(temp != NULL)
    {
        if(temp->data == packet)
            return temp;

        temp = temp->next;
    }

    return NULL;
}

int minimum(int a,int b)
{
    return (a < b) ? a : b;
}

int main()
{
    int sock;
    struct sockaddr_in receiver;

    socklen_t len = sizeof(receiver);

    int tp,tt,loss;
    int total_time = 0;
    int total_transmissions = 0;

    int total_packets;
    int window_size;

    float alpha;

    int maximum_packets;
    int number_of_sequence_bits;
    int sequence_number_range;

    /*
       INPUT
    */

    printf("Enter Transmission Time: ");
    scanf("%d",&tt);

    printf("Enter Propagation Time: ");
    scanf("%d",&tp);

    printf("Enter loss interval (every Nth packet lost once): ");
    scanf("%d",&loss);

    if(loss < 2)
        loss = 2;

    /*
       FORMULA CALCULATIONS
    */

    alpha = (float)tp / tt;

    maximum_packets = 1 + (int)(2 * alpha);

    number_of_sequence_bits =
        (int)floor(log2(maximum_packets));

    sequence_number_range =
        (int)pow(2,number_of_sequence_bits);

    /*
       SELECTIVE REPEAT:
       W <= 2^(n-1)
    */

    window_size =
        minimum(maximum_packets,
                sequence_number_range / 2);

    total_packets = maximum_packets;

    /*
       DISPLAY PARAMETERS
    */

    printf("\n========== SELECTIVE REPEAT PARAMETERS ==========\n");

    printf("Transmission Time       : %d\n",tt);
    printf("Propagation Time        : %d\n",tp);
    printf("Alpha                   : %.2f\n",alpha);
    printf("Maximum Packets         : %d\n",maximum_packets);
    printf("Number of Sequence Bits : %d\n",
           number_of_sequence_bits);
    printf("Sequence Number Range   : %d\n",
           sequence_number_range);
    printf("SR Window Size          : %d\n",
           window_size);
    printf("Loss Interval           : %d\n",loss);

    printf("=================================================\n\n");

    /*
       SOCKET
    */

    receiver.sin_family = AF_INET;
    receiver.sin_port = htons(9000);
    receiver.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    sock = socket(AF_INET,SOCK_DGRAM,0);

    if(sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    /*
       SEND PARAMETERS
    */

    sendto(sock,
           &total_packets,
           sizeof(total_packets),
           0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    sendto(sock,
           &window_size,
           sizeof(window_size),
           0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    sendto(sock,
           &tt,
           sizeof(tt),
           0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    sendto(sock,
           &tp,
           sizeof(tp),
           0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    sendto(sock,
           &loss,
           sizeof(loss),
           0,
           (struct sockaddr *)&receiver,
           sizeof(receiver));

    /*
       TIMEOUT
    */

    struct timeval timeout;

    timeout.tv_sec = tt + 2 * tp;
    timeout.tv_usec = 0;

    setsockopt(sock,
               SOL_SOCKET,
               SO_RCVTIMEO,
               &timeout,
               sizeof(timeout));

    queue *window = createQueue();

    int base = 1;
    int next = 1;
    int cycle = 1;

    /*
       ACKED ARRAY
    */

    int acked[100000];

    for(int i=0;i<100000;i++)
        acked[i] = 0;

    /*
       MAIN LOOP
    */

    while(base <= total_packets)
    {
        int relseq = 0;

        /*
           SEND NEW PACKETS
        */

        while(next <= total_packets &&
              next < base + window_size)
        {
            sleep(tt);

            total_time += tt;

            sendto(sock,
                   &next,
                   sizeof(next),
                   0,
                   (struct sockaddr *)&receiver,
                   sizeof(receiver));

            Enqueue(window,
                    next,
                    cycle,
                    relseq++);

            total_transmissions++;

            printf("Timer: %d\tCycle: %d\tBase: %d\tNext: %d\tSent packet %d\n",
                   total_time,
                   cycle,
                   base,
                   next,
                   next);

            printQueue(window);

            next++;
        }

        /*
           WAIT FOR ACK
        */

        int ack;

        int result =
            recvfrom(sock,
                     &ack,
                     sizeof(ack),
                     0,
                     (struct sockaddr *)&receiver,
                     &len);

        /*
           ACK RECEIVED
        */

        if(result > 0)
        {
            total_time += tp;

            acked[ack] = 1;

            node *temp = findPacket(window,ack);

            if(temp != NULL)
                temp->acked = 1;

            printf("Timer: %d\tCycle: %d\tACK %d received (individual)\n",
                   total_time,
                   cycle,
                   ack);

            /*
               REMOVE ONLY THIS PACKET
            */

            removePacket(window,ack);

            /*
               SLIDE BASE ONLY WHEN THE
               BASE PACKET HAS BEEN ACKED
            */

            while(base <= total_packets &&
                  acked[base] == 1)
            {
                base++;
            }

            printQueue(window);

            cycle++;
        }

        /*
           TIMEOUT
        */

        else
        {
            /*
               FIND FIRST UNACKNOWLEDGED
               PACKET
            */

            node *temp = window->front;

            if(temp != NULL)
            {
                printf("Timer: %d\tCycle: %d\tTimeout! Selective Repeat: retransmitting packet %d\n",
                       total_time,
                       cycle,
                       temp->data);

                sleep(tt);

                total_time += tt;

                sendto(sock,
                       &temp->data,
                       sizeof(int),
                       0,
                       (struct sockaddr *)&receiver,
                       sizeof(receiver));

                temp->cycle = cycle + 1;

                total_transmissions++;

                printf("Timer: %d\tCycle: %d\tRetransmitted packet %d\n",
                       total_time,
                       cycle + 1,
                       temp->data);

                printQueue(window);

                cycle++;
            }
        }
    }

    /*
       FINAL OUTPUT
    */

    printf("\nAll packets transmitted successfully.\n");

    printf("Total time required: %d\n",
           total_time);

    printf("Total number of transmissions (including retransmissions): %d\n",
           total_transmissions);

    close(sock);

    return 0;
}