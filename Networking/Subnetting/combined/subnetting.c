
#include <stdio.h>
#include <string.h>

unsigned int ipToInt(char ip[])
{
    unsigned int a, b, c, d;

    sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d);

    return (a << 24) | (b << 16) | (c << 8) | d;
}

void intToIP(unsigned int ip, char str[])
{
    sprintf(str, "%u.%u.%u.%u",
            (ip >> 24) & 255,
            (ip >> 16) & 255,
            (ip >> 8) & 255,
            ip & 255);
}

void displaySubnet(unsigned int network, int prefix)
{
    unsigned int mask, broadcast;
    char netIP[20], maskIP[20];
    char firstIP[20], lastIP[20], broadIP[20];

    if (prefix == 0)
        mask = 0;
    else
        mask = 0xFFFFFFFFU << (32 - prefix);

    broadcast = network | ~mask;

    intToIP(network, netIP);
    intToIP(mask, maskIP);
    intToIP(broadcast, broadIP);

    printf("Network Address: %s/%d\n", netIP, prefix);
    printf("Subnet Mask: %s\n", maskIP);

    if (prefix <= 30)
    {
        intToIP(network + 1, firstIP);
        intToIP(broadcast - 1, lastIP);

        printf("Usable Host Range: %s - %s\n",
               firstIP, lastIP);
    }
    else
    {
        printf("No conventional usable host range\n");
    }

    printf("Broadcast Address: %s\n", broadIP);
}

int main()
{
    int choice, prefix, n, i, j;
    int hosts[100], bits[100], temp;
    char ip[20];

    unsigned int network, mask;
    unsigned int current, blockSize;

    printf("1. FLSM\n");
    printf("2. VLSM\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);

    if (choice != 1 && choice != 2)
    {
        printf("Invalid choice. Select 1 or 2.\n");
        return 1;
    }

    printf("Enter Network IP Address: ");
    scanf("%19s", ip);

    printf("Enter CIDR Prefix (e.g. 24 for /24): ");
    scanf("%d", &prefix);

    if (prefix < 0 || prefix > 30)
    {
        printf("Enter a CIDR prefix between 0 and 30.\n");
        return 1;
    }

    network = ipToInt(ip);

    if (prefix == 0)
        mask = 0;
    else
        mask = 0xFFFFFFFFU << (32 - prefix);

    network = network & mask;

    switch (choice)
    {
        case 1:
        {
            int borrowed = 0;
            int newPrefix;

            printf("Enter Number of Subnets: ");
            scanf("%d", &n);

            if (n < 1 || n > 100)
            {
                printf("Invalid number of subnets.\n");
                return 1;
            }

            while ((1U << borrowed) < (unsigned int)n)
                borrowed++;

            newPrefix = prefix + borrowed;

            if (newPrefix > 30)
            {
                printf("Too many subnets for usable host allocation.\n");
                return 1;
            }

            blockSize = 1U << (32 - newPrefix);
            current = network;

            for (i = 0; i < n; i++)
            {
                printf("\n--- Subnet %d ---\n", i + 1);

                displaySubnet(current, newPrefix);

                current += blockSize;
            }

            break;
        }

        case 2:
        {
            printf("Enter Number of Subnets: ");
            scanf("%d", &n);

            if (n < 1 || n > 100)
            {
                printf("Invalid number of subnets.\n");
                return 1;
            }

            for (i = 0; i < n; i++)
            {
                printf("Enter Required Hosts for Subnet %d: ",
                       i + 1);
                scanf("%d", &hosts[i]);

                if (hosts[i] < 1)
                {
                    printf("Invalid host requirement.\n");
                    return 1;
                }

                bits[i] = 0;

                while ((1U << bits[i]) <
                       (unsigned int)hosts[i] + 2U)
                {
                    bits[i]++;

                    if (bits[i] >= 31)
                    {
                        printf("Host requirement is too large.\n");
                        return 1;
                    }
                }

                bits[i] = 32 - bits[i];
            }

            /* Sort by descending host requirement */
            for (i = 0; i < n - 1; i++)
            {
                for (j = i + 1; j < n; j++)
                {
                    if (hosts[j] > hosts[i])
                    {
                        temp = hosts[i];
                        hosts[i] = hosts[j];
                        hosts[j] = temp;

                        temp = bits[i];
                        bits[i] = bits[j];
                        bits[j] = temp;
                    }
                }
            }

            current = network;

            for (i = 0; i < n; i++)
            {
                int newPrefix = bits[i];
                unsigned long long endOfNetwork;
                unsigned long long endOfSubnet;

                if (newPrefix < prefix || newPrefix > 30)
                {
                    printf("\nSubnet %d cannot fit in the network.\n",
                           i + 1);
                    return 1;
                }

                blockSize = 1U << (32 - newPrefix);

                /* Align the address to the subnet boundary */
                current = (current + blockSize - 1) &
                          ~(blockSize - 1);

                endOfNetwork = (unsigned long long)network +
                               (1ULL << (32 - prefix));

                endOfSubnet = (unsigned long long)current +
                              blockSize;

                if (endOfSubnet > endOfNetwork)
                {
                    printf("\nNot enough address space for Subnet %d.\n",
                           i + 1);
                    return 1;
                }

                printf("\n--- Subnet %d (Required Hosts: %d) ---\n",
                       i + 1, hosts[i]);

                displaySubnet(current, newPrefix);

                current += blockSize;
            }

            break;
        }
    }

    return 0;
}
