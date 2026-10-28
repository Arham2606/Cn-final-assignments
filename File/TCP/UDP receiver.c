
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdint.h>

#define PORT 8080
#define SIZE 1024

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

int main()
{
    int server_sock, n;
    struct sockaddr_in server, client;
    socklen_t client_len = sizeof(client);
    struct packet p, ack;
    char filename[100];
    FILE *fp;
    uint32_t expected = 1;
    int filetype = 0;

    server_sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&server,
             sizeof(server)) < 0) {
        perror("Bind failed");
        close(server_sock);
        return 1;
    }

    printf("Receiver waiting for sender...\n");

    /* Receive the file type */
    while (1) {
        client_len = sizeof(client);

        n = recvfrom(server_sock, &p, sizeof(p), 0,
                     (struct sockaddr *)&client, &client_len);

        if (n < (int)(sizeof(p.kind) + sizeof(p.filetype) +
                      sizeof(p.seq) + sizeof(p.len))) {
            continue;
        }

        if (p.kind == TYPE && p.seq == 0 &&
            p.len == 0 &&
            p.filetype >= 1 && p.filetype <= 3) {
            filetype = p.filetype;
            break;
        }
    }

    printf("Sender connected\n");

    switch (filetype) {
        case 1:
            strcpy(filename, "received.txt");
            break;
        case 2:
            strcpy(filename, "received.mp3");
            break;
        case 3:
            strcpy(filename, "received.mp4");
            break;
        default:
            printf("Invalid file type received\n");
            close(server_sock);
            return 1;
    }

    fp = fopen(filename, "wb");

    if (fp == NULL) {
        perror("Output file opening failed");
        close(server_sock);
        return 1;
    }

    /* Acknowledge the file type */
    memset(&ack, 0, sizeof(ack));
    ack.kind = ACK;
    ack.seq = 0;

    sendto(server_sock, &ack, sizeof(ack), 0,
           (struct sockaddr *)&client, client_len);

    printf("Receiving file: %s\n", filename);

    /* Receive file data until END arrives */
    while (1) {
        client_len = sizeof(client);

        n = recvfrom(server_sock, &p, sizeof(p), 0,
                     (struct sockaddr *)&client, &client_len);

        if (n < (int)(sizeof(p.kind) + sizeof(p.filetype) +
                      sizeof(p.seq) + sizeof(p.len))) {
            continue;
        }

        memset(&ack, 0, sizeof(ack));
        ack.kind = ACK;
        ack.seq = p.seq;

        if (p.kind == DATA) {
            if (p.len > SIZE)
                continue;

            if (p.seq == expected) {
                if (fwrite(p.data, 1, p.len, fp) != p.len) {
                    perror("Writing file failed");
                    fclose(fp);
                    close(server_sock);
                    return 1;
                }

                expected++;
            } else if (p.seq > expected) {
                /* A packet is missing; do not acknowledge it */
                continue;
            }

            /* ACK expected packet or a duplicate */
            sendto(server_sock, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&client, client_len);
        }
        else if (p.kind == END && p.seq == expected) {
            sendto(server_sock, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&client, client_len);
            break;
        }
    }

    fclose(fp);

    printf("File received successfully: %s\n", filename);

    close(server_sock);
    return 0;
}
