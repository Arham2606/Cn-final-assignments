#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAX_SIZE 100

int find_length(char str[]) {
    int i = 0;
    while (str[i] != '\0') i++;
    return i;
}

int is_parity_position(int position) {
    int power = 1;
    for (int i = 0; i < 32; i++) {
        if (position > 0 && power == position) return 1;
        else power = power * 2;
    }
    return 0;
}

int calculate_parity_bits(int total_bits) {
    int parity_bits = 0;
    for (int r = 0; r <= 20; r++) {
        int power = 1;
        for (int j = 0; j < r; j++) power = power * 2;
        if (power > total_bits) break;
        else parity_bits = r + 1;
    }
    return parity_bits;
}

int check_parity(int *hamming, int total_bits, int parity_position) {
    int count = 0;
    for (int position = parity_position; position <= total_bits; position++) {
        int offset = position - parity_position;
        int block = offset / parity_position;
        int remainder = block % 2;
        if (remainder == 0 && hamming[position] == 1) count++;
    }
    return (count % 2 == 0) ? 0 : 1;
}

void print_hamming(int *hamming, int total_bits) {
    printf("\nPosition : ");
    for (int i = 1; i <= total_bits; i++) printf("%d ", i);
    printf("\nHamming  : ");
    for (int i = 1; i <= total_bits; i++) printf("%d ", hamming[i]);
    printf("\n");
}

int main() {
    int sockfd;
    struct sockaddr_in receiver_address, transmitter_address;
    socklen_t address_length;
    char received_data[MAX_SIZE];
    int total_bits, parity_bits;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    receiver_address.sin_family = AF_INET;
    receiver_address.sin_port = htons(PORT);
    receiver_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&receiver_address, sizeof(receiver_address)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    printf("Receiver waiting for Hamming code...\n");

    address_length = sizeof(transmitter_address);
    int received_length = recvfrom(sockfd, received_data, MAX_SIZE - 1, 0, (struct sockaddr *)&transmitter_address, &address_length);
    if (received_length < 0) {
        perror("Receive failed");
        close(sockfd);
        return 1;
    }

    received_data[received_length] = '\0';
    printf("\nHamming code received = %s\n", received_data);

    total_bits = find_length(received_data);

    int *arr = malloc((total_bits + 1) * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        close(sockfd);
        return 1;
    }
    for (int i = 1; i <= total_bits; i++) arr[i] = received_data[i - 1] - '0';

    printf("\nReceived Hamming Code:");
    print_hamming(arr, total_bits);

    parity_bits = calculate_parity_bits(total_bits);

    int altered_bit = 0;
    for (int i = 0; i < parity_bits; i++) {
        int parity_position = 1;
        for (int j = 0; j < i; j++) parity_position = parity_position * 2;
        int error = check_parity(arr, total_bits, parity_position);
        printf("P%d check = %d\n", parity_position, error);
        if (error == 1) altered_bit += parity_position;
    }

    printf("\naltered_bit = %d\n", altered_bit);

    if (altered_bit == 0) {
        printf("\nNo error detected.\n");
    } else {
        printf("\nError detected at position %d.\n", altered_bit);
        arr[altered_bit] = !arr[altered_bit];
        printf("Bit corrected.\n");
        print_hamming(arr, total_bits);
    }

    int data_bits = total_bits - parity_bits;
    int data_index = 1;
    for (int position = 1; position <= total_bits; position++) {
        if (is_parity_position(position)) continue;
        else {
            arr[data_index] = arr[position];
            data_index++;
        }
    }

    int *temp = realloc(arr, (data_bits + 1) * sizeof(int));
    if (temp == NULL) {
        printf("Memory reallocation failed.\n");
        free(arr);
        close(sockfd);
        return 1;
    }
    arr = temp;

    char original_data[MAX_SIZE];
    for (int i = 1; i <= data_bits; i++) original_data[i - 1] = arr[i] + '0';
    original_data[data_bits] = '\0';

    printf("\nOriginal Data = %s\n", original_data);

    free(arr);
    close(sockfd);
    return 0;
}
