#include <stdio.h>

#define INF 9999
#define MAX 62

char router[MAX];

int get_index(char name, int n) {
    for (int i = 0; i < n; i++) {
        if (router[i] == name)
            return i;
    }
    return -1;
}

void print_table(int n, int dist[], int parent[]) {
    printf("  V | d    | p\n");
    for (int i = 0; i < n; i++) {
        printf("  %c | ", router[i]);
        if (dist[i] == INF)
            printf("%-4s | ", "INF");
        else
            printf("%-4d | ", dist[i]);
        if (parent[i] == -1)
            printf("-\n");
        else
            printf("%c\n", router[parent[i]]);
    }
}

void print_path(int parent[], int v) {
    if (parent[v] == -1) {
        printf("%c", router[v]);
        return;
    }
    print_path(parent, parent[v]);
    printf(" -> %c", router[v]);
}

int main(void) {
    int n, m;
    int cost[MAX][MAX], dist[MAX], parent[MAX];

    printf("Enter number of routers: ");
    scanf("%d", &n);

    printf("Enter router names: ");
    for (int i = 0; i < n; i++)
        scanf(" %c", &router[i]);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            cost[i][j] = INF;

    printf("Enter number of links: ");
    scanf("%d", &m);

    printf("Enter each link as: router1 router2 cost\n");
    for (int i = 0; i < m; i++) {
        char a, b;
        int w;
        scanf(" %c %c %d", &a, &b, &w);
        int u = get_index(a, n);
        int v = get_index(b, n);
        cost[u][v] = w;
    }

    char src_name;
    printf("Enter source router: ");
    scanf(" %c", &src_name);
    int src = get_index(src_name, n);

    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        parent[i] = -1;
    }
    dist[src] = 0;

    printf("\nInitial table\n");
    print_table(n, dist, parent);

    for (int iter = 1; iter <= n - 1; iter++) {
        for (int u = 0; u < n; u++) {
            for (int v = 0; v < n; v++) {
                if (cost[u][v] != INF && dist[u] != INF && dist[v] > dist[u] + cost[u][v]) {
                    dist[v] = dist[u] + cost[u][v];
                    parent[v] = u;
                }
            }
        }

        printf("\nAfter iteration %d\n", iter);
        print_table(n, dist, parent);
    }

    int cycle = 0;
    for (int u = 0; u < n; u++) {
        for (int v = 0; v < n; v++) {
            if (cost[u][v] != INF && dist[u] != INF && dist[v] > dist[u] + cost[u][v])
                cycle = 1;
        }
    }

    if (cycle) {
        printf("\nNegative weight cycle detected\n");
        return 0;
    }

    printf("\nNo negative weight cycle\n");
    printf("\nFinal table (source %c)\n", src_name);
    printf("Router | Distance | Path\n");

    for (int i = 0; i < n; i++) {
        printf("%-6c | ", router[i]);
        if (dist[i] == INF) {
            printf("%-8s | unreachable\n", "INF");
        } else {
            printf("%-8d | ", dist[i]);
            print_path(parent, i);
            printf("\n");
        }
    }

    return 0;
}
