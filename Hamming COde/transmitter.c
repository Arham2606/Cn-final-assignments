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

int calculate_parity_bits(int data_bits) {
    int parity_bits = 0;
    for (int r = 0; r <= 20; r++) {
        int capacity = 1;
        for (int j = 0; j < r; j++) capacity = capacity * 2;
        if (capacity < data_bits + r + 1) continue;
        else {
            parity_bits = r;
            break;
        }
    }
    return parity_bits;
}

void calculate_parity(int *hamming, int total_bits, int parity_position) {
    int count = 0;
    for (int position = parity_position; position <= total_bits; position++) {
        int offset = position - parity_position;
        int block = offset / parity_position;
        int remainder = block % 2;
        if (remainder == 0 && hamming[position] == 1) count++;
    }
    hamming[parity_position] = (count % 2 == 1) ? 1 : 0;
}

void print_hamming(int *hamming, int total_bits) {
    printf("\nPosition : ");
    for (int i = 1; i <= total_bits; i++) printf("%d ", i);
    printf("\nHamming  : ");
    for (int i = 1; i <= total_bits; i++) printf("%d ", hamming[i]);
    printf("\n");
}

void array_to_string(int *hamming, int total_bits, char result[]) {
    for (int i = 1; i <= total_bits; i++) result[i - 1] = hamming[i] + '0';
    result[total_bits] = '\0';
}

int main() {
    int sockfd;
    struct sockaddr_in receiver_address;
    char input[MAX_SIZE], final_data[MAX_SIZE];
    int data_bits, parity_bits, total_bits;

    printf("Enter binary data: ");
    scanf("%99s", input);

    data_bits = find_length(input);

    for (int i = 0; i < data_bits; i++) {
        if (input[i] != '0' && input[i] != '1') {
            printf("Invalid binary data.\n");
            return 1;
        }
    }

    int *arr = malloc((data_bits + 1) * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
    }
    for (int i = 1; i <= data_bits; i++) arr[i] = input[i - 1] - '0';

    parity_bits = calculate_parity_bits(data_bits);
    total_bits = data_bits + parity_bits;

    printf("\nData bits = %d", data_bits);
    printf("\nParity bits = %d", parity_bits);
    printf("\nTotal bits = %d\n", total_bits);

    int *temp = realloc(arr, (total_bits + 1) * sizeof(int));
    if (temp == NULL) {
        printf("Memory reallocation failed.\n");
        free(arr);
        return 1;
    }
    arr = temp;

    int data_index = data_bits;
    for (int position = total_bits; position >= 1; position--) {
        if (is_parity_position(position)) {
            arr[position] = 0;
        } else {
            arr[position] = arr[data_index];
            data_index--;
        }
    }

    printf("\nAfter shifting data (parity slots = 0):");
    print_hamming(arr, total_bits);

    for (int i = 0; i < parity_bits; i++) {
        int parity_position = 1;
        for (int j = 0; j < i; j++) parity_position = parity_position * 2;
        calculate_parity(arr, total_bits, parity_position);
    }

    printf("\nFinal Hamming Code:");
    print_hamming(arr, total_bits);

    array_to_string(arr, total_bits, final_data);
    printf("\nFinal Hamming code = %s\n", final_data);

    int error_position;
    printf("\nEnter bit position to alter (0 for no error): ");
    scanf("%d", &error_position);

    if (error_position < 0 || error_position > total_bits) {
        printf("Invalid position.\n");
        free(arr);
        return 1;
    }

    if (error_position != 0) {
        arr[error_position] = !arr[error_position];
        printf("\nBit at position %d altered.", error_position);
        array_to_string(arr, total_bits, final_data);
        printf("\nHamming code after alteration = %s\n", final_data);
    } else {
        printf("\nNo bit altered.\n");
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        free(arr);
        return 1;
    }

    receiver_address.sin_family = AF_INET;
    receiver_address.sin_port = htons(PORT);
    receiver_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    sendto(sockfd, final_data, find_length(final_data) + 1, 0, (struct sockaddr *)&receiver_address, sizeof(receiver_address));

    printf("\nHamming code sent through UDP.\n");

    free(arr);
    close(sockfd);
    return 0;
}
