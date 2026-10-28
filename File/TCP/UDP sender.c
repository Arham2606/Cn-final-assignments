
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdint.h>

#define PORT 8080
#define SIZE 1024
#define RETRIES 5

#define TYPE 1
#define DATA 2
#define END  3
#define ACK  4

struct packet {
    int kind;
    int filetype;
    uint32_t seq;
    uint16_t len;
    char data[SIZE];
};

int send_wait_ack(int sock, struct sockaddr_in *server,
                  struct packet *p)
{
    socklen_t addrlen = sizeof(*server);
    struct packet ack;
    int attempt;

    for (attempt = 0; attempt < RETRIES; attempt++) {
        if (sendto(sock, p, sizeof(*p), 0,
                   (struct sockaddr *)server,
                   addrlen) < 0) {
            perror("Sending packet failed");
            return 0;
        }

        while (1) {
            int n = recvfrom(sock, &ack, sizeof(ack), 0,
                             NULL, NULL);

            if (n < 0)
                break;

            if (n >= (int)(sizeof(ack.kind) +
                           sizeof(ack.filetype) +
                           sizeof(ack.seq) +
                           sizeof(ack.len)) &&
                ack.kind == ACK && ack.seq == p->seq) {
                return 1;
            }
        }

        printf("No acknowledgement. Retrying...\n");
    }

    printf("Transmission failed after retries.\n");
    return 0;
}

int main()
{
    int sock, choice, n;
    struct sockaddr_in server;
    struct packet p;
    char filename[100];
    char buffer[SIZE];
    FILE *fp;

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    /* Set a 1-second timeout for acknowledgements */
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
               &timeout, sizeof(timeout));

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Select file type:\n");
    printf("1. Text file\n");
    printf("2. Audio file (MP3)\n");
    printf("3. Video file (MP4)\n");
    printf("Enter choice: ");

    if (scanf("%d", &choice) != 1) {
        printf("Invalid input\n");
        close(sock);
        return 1;
    }

    switch (choice) {
        case 1:
            strcpy(filename, "text.txt");
            break;
        case 2:
            strcpy(filename, "audio.mp3");
            break;
        case 3:
            strcpy(filename, "video.mp4");
            break;
        default:
            printf("Invalid choice\n");
            close(sock);
            return 1;
    }

    fp = fopen(filename, "rb");

    if (fp == NULL) {
        perror("File opening failed");
        close(sock);
        return 1;
    }

    /* Send file type */
    memset(&p, 0, sizeof(p));
    p.kind = TYPE;
    p.filetype = choice;
    p.seq = 0;
    p.len = 0;

    if (!send_wait_ack(sock, &server, &p)) {
        fclose(fp);
        close(sock);
        return 1;
    }

    printf("Sending %s...\n", filename);

    /* Read and send file contents */
    uint32_t seq = 1;

    while ((n = fread(buffer, 1, SIZE, fp)) > 0) {
        memset(&p, 0, sizeof(p));
        p.kind = DATA;
        p.seq = seq;
        p.len = (uint16_t)n;
        memcpy(p.data, buffer, n);

        if (!send_wait_ack(sock, &server, &p)) {
            fclose(fp);
            close(sock);
            return 1;
        }

        seq++;
    }

    if (ferror(fp)) {
        perror("File reading failed");
        fclose(fp);
        close(sock);
        return 1;
    }

    fclose(fp);

    /* Send end-of-file packet */
    memset(&p, 0, sizeof(p));
    p.kind = END;
    p.seq = seq;
    p.len = 0;

    if (!send_wait_ack(sock, &server, &p)) {
        close(sock);
        return 1;
    }

    printf("%s sent successfully\n", filename);

    close(sock);
    return 0;
}
