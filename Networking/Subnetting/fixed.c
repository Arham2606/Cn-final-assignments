#include <stdio.h>

void decimal_to_bits(int num, int bits[], int start) {
    for (int i = 7; i >= 0; i--) {
        bits[start + i] = num % 2;
        num = num / 2;
    }
}

int bits_to_decimal(int bits[], int start) {
    int value = 0;
    for (int i = 0; i < 8; i++) {
        value = value * 2 + bits[start + i];
    }
    return value;
}

void ip_to_bits(int octet[], int bits[]) {
    for (int i = 0; i < 4; i++) {
        decimal_to_bits(octet[i], bits, i * 8);
    }
}

int and_bit(int x, int y) {
    if (x == 1 && y == 1) {
        return 1;
    }
    return 0;
}

void make_mask(int prefix, int mask[]) {
    for (int i = 0; i < 32; i++) {
        if (i < prefix) {
            mask[i] = 1;
        } else {
            mask[i] = 0;
        }
    }
}

void print_ip(int bits[]) {
    printf("%d.%d.%d.%d",
           bits_to_decimal(bits, 0), bits_to_decimal(bits, 8),
           bits_to_decimal(bits, 16), bits_to_decimal(bits, 24));
}

long long power_of_2(int n) {
    long long result = 1;
    for (int i = 0; i < n; i++) {
        result = result * 2;
    }
    return result;
}

int main(void) {
    int octet[4], prefix;

    printf("Enter network: ");
    scanf("%d.%d.%d.%d/%d", &octet[0], &octet[1], &octet[2], &octet[3], &prefix);

    int new_prefix = prefix + 3;
    int ip[32], old_mask[32], new_mask[32], network[32];

    ip_to_bits(octet, ip);
    make_mask(prefix, old_mask);
    make_mask(new_prefix, new_mask);

    for (int i = 0; i < 32; i++) {
        network[i] = and_bit(ip[i], old_mask[i]);
    }

    long long total = power_of_2(32 - new_prefix);

    printf("\nOriginal network: ");
    print_ip(network);
    printf("/%d\nNew subnet mask : ", prefix);
    print_ip(new_mask);
    printf(" (/%d)\nHosts per subnet: %lld\n", new_prefix, total - 2);

    for (int s = 0; s < 8; s++) {
        int first[32], last[32], usable_first[32], usable_last[32];
        int sub_bits[3];
        int temp = s;

        for (int k = 2; k >= 0; k--) {
            sub_bits[k] = temp % 2;
            temp = temp / 2;
        }

        for (int i = 0; i < 32; i++) {
            if (i < prefix) {
                first[i] = network[i];
                last[i] = network[i];
            } else if (i < new_prefix) {
                first[i] = sub_bits[i - prefix];
                last[i] = sub_bits[i - prefix];
            } else {
                first[i] = 0;
                last[i] = 1;
            }
            usable_first[i] = first[i];
            usable_last[i] = last[i];
        }
        usable_first[31] = 1;
        usable_last[31] = 0;

        printf("\nSubnet %d\n", s + 1);
        printf("  1st IP       : ");
        print_ip(first);
        printf("\n  Last IP      : ");
        print_ip(last);
        printf("\n  Usable range : ");
        print_ip(usable_first);
        printf(" - ");
        print_ip(usable_last);
        printf("\n  Subnet mask  : ");
        print_ip(new_mask);
        printf("\n");
    }
    return 0;
}
