#include <stdio.h>
#include <ctype.h>

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

void print_path(int parent[], int v) {
    if (parent[v] == -1) {
        printf("%c", router[v]);
        return;
    }
    print_path(parent, parent[v]);
    printf(" -> %c", router[v]);
}

void print_row(int step, char selected, int stack[], int count, int dist[], int n, int width) {
    char buf[2 * MAX + 8];
    int k = 0;

    if (count == 0) {
        buf[k++] = '-';
    } else {
        for (int i = 0; i < count; i++) {
            buf[k++] = router[stack[i]];
            buf[k++] = ' ';
        }
    }
    buf[k] = '\0';

    printf("%-4d | %-8c | %-*s |", step, selected, width, buf);
    for (int i = 0; i < n; i++) {
        if (dist[i] == INF)
            printf(" %4s ", "INF");
        else
            printf(" %4d ", dist[i]);
    }
    printf("\n");
}

int main(void) {
    int n, m;
    int cost[MAX][MAX];
    int dist[MAX], parent[MAX], visited[MAX];
    int stack[MAX], count = 0;

    printf("Enter number of routers: ");
    scanf("%d", &n);

    printf("Enter router names: ");
    for (int i = 0; i < n; i++) {
        scanf(" %c", &router[i]);
    }

    printf("Enter number of links: ");
    scanf("%d", &m);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            cost[i][j] = (i == j) ? 0 : INF;

    printf("Enter each link: router1 router2 cost\n");
    for (int i = 0; i < m; i++) {
        char a, b;
        int w;

        scanf(" %c %c %d", &a, &b, &w);

        int u = get_index(a, n);
        int v = get_index(b, n);

        cost[u][v] = w;
        cost[v][u] = w;
    }

    char src_name;
    printf("Enter source router: ");
    scanf(" %c", &src_name);

    int src = get_index(src_name, n);

    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        parent[i] = -1;
        visited[i] = 0;
    }
    dist[src] = 0;

    int width = 2 * n;
    if (width < 7) width = 7;

    printf("\nStep | Selected | %-*s |", width, "Stack");
    for (int i = 0; i < n; i++)
        printf(" D(%c) ", router[i]);
    printf("\n");

    print_row(0, '-', stack, count, dist, n, width);

    for (int step = 1; step <= n; step++) {
        int u = -1;

        for (int i = 0; i < n; i++) {
            if (!visited[i] && dist[i] != INF) {
                if (u == -1 || dist[i] < dist[u])
                    u = i;
            }
        }

        if (u == -1)
            break;

        visited[u] = 1;
        stack[count++] = u;

        for (int v = 0; v < n; v++) {
            if (!visited[v] && cost[u][v] != INF) {
                if (dist[u] + cost[u][v] < dist[v]) {
                    dist[v] = dist[u] + cost[u][v];
                    parent[v] = u;
                }
            }
        }

        print_row(step, router[u], stack, count, dist, n, width);
    }

    printf("\nShortest paths from router %c\n", src_name);
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
