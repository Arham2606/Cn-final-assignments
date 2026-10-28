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

void make_subnet(int network[], int prefix, int b, int number, int first[], int last[]) {
    int sub_bits[32];

    for (int k = b - 1; k >= 0; k--) {
        sub_bits[k] = number % 2;
        number = number / 2;
    }

    for (int i = 0; i < 32; i++) {
        if (i < prefix) {
            first[i] = network[i];
            last[i] = network[i];
        } else if (i < prefix + b) {
            first[i] = sub_bits[i - prefix];
            last[i] = sub_bits[i - prefix];
        } else {
            first[i] = 0;
            last[i] = 1;
        }
    }
}

void print_subnet(int first[], int last[], int sub_prefix, long long hosts) {
    int usable_first[32], usable_last[32], mask[32];

    for (int i = 0; i < 32; i++) {
        usable_first[i] = first[i];
        usable_last[i] = last[i];
    }
    usable_first[31] = 1;
    usable_last[31] = 0;

    printf("  Hosts given  : %lld\n", hosts);
    printf("  1st IP       : ");
    print_ip(first);
    printf("\n  Last IP      : ");
    print_ip(last);
    printf("\n  Usable range : ");
    print_ip(usable_first);
    printf(" - ");
    print_ip(usable_last);

    make_mask(sub_prefix, mask);
    printf("\n  Subnet mask  : ");
    print_ip(mask);
    printf(" (/%d)\n", sub_prefix);
}

int main(void) {
    int octet[4], prefix, n;

    printf("Enter network: ");
    scanf("%d.%d.%d.%d/%d", &octet[0], &octet[1], &octet[2], &octet[3], &prefix);

    printf("Enter number of subnets: ");
    scanf("%d", &n);

    int ip[32], old_mask[32], network[32];

    ip_to_bits(octet, ip);
    make_mask(prefix, old_mask);

    for (int i = 0; i < 32; i++) {
        network[i] = and_bit(ip[i], old_mask[i]);
    }

    int b = 0;
    while (power_of_2(b) < n) {
        b++;
    }

    int new_prefix = prefix + b;
    long long total_parts = power_of_2(b);
    long long part_size = power_of_2(32 - new_prefix);

    printf("\nOriginal network: ");
    print_ip(network);
    printf("/%d\n", prefix);
    printf("Borrowed bits   : %d (%lld equal parts of /%d)\n",
           b, total_parts, new_prefix);

    for (int s = 0; s < n - 1; s++) {
        int first[32], last[32];

        make_subnet(network, prefix, b, s, first, last);

        printf("\nSubnet %d (1 part)\n", s + 1);
        print_subnet(first, last, new_prefix, part_size - 2);
    }

    int first[32], last[32], temp[32];
    long long remaining = total_parts - (n - 1);

    make_subnet(network, prefix, b, n - 1, first, temp);
    make_subnet(network, prefix, b, total_parts - 1, temp, last);

    int k = 0;
    while (power_of_2(k) < remaining) {
        k++;
    }

    printf("\nSubnet %d (%lld parts)\n", n, remaining);
    if (power_of_2(k) == remaining) {
        print_subnet(first, last, new_prefix - k, remaining * part_size - 2);
    } else {
        print_subnet(first, last, -1, remaining * part_size - 2);
    }
    return 0;
}
