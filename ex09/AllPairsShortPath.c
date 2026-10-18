#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VERTICES 10
#define EDGES 20
#define MIN_WEIGHT 1
#define MAX_WEIGHT 100
#define INF 1000000000

typedef struct {
    int u;
    int v;
    int weight;
} Edge;

static int random_weight(void)
{
    return rand() % (MAX_WEIGHT - MIN_WEIGHT + 1) + MIN_WEIGHT;
}

static void initialize_matrix(int matrix[VERTICES][VERTICES])
{
    int i, j;

    for (i = 0; i < VERTICES; i++) {
        for (j = 0; j < VERTICES; j++) {
            if (i == j)
                matrix[i][j] = 0;
            else
                matrix[i][j] = INF;
        }
    }
}

static void add_edge(Edge edges[], int *count,
                     int matrix[VERTICES][VERTICES],
                     int u, int v, int weight)
{
    edges[*count].u = u;
    edges[*count].v = v;
    edges[*count].weight = weight;
    (*count)++;

    matrix[u][v] = weight;
    matrix[v][u] = weight;
}

static void generate_connected_graph(
    Edge edges[EDGES],
    int matrix[VERTICES][VERTICES]
)
{
    int count = 0;
    int i;
    int u;
    int v;

    initialize_matrix(matrix);

    /* 먼저 신장 트리를 생성하여 그래프의 연결성을 보장한다. */
    for (i = 1; i < VERTICES; i++) {
        u = rand() % i;
        v = i;
        add_edge(edges, &count, matrix, u, v, random_weight());
    }

    /* 이후 중복되지 않는 간선을 추가한다. */
    while (count < EDGES) {
        u = rand() % VERTICES;
        v = rand() % VERTICES;

        if (u == v || matrix[u][v] != INF)
            continue;

        add_edge(edges, &count, matrix, u, v, random_weight());
    }
}

static void print_graph(const Edge edges[EDGES])
{
    int i;

    printf("Graph\n");
    printf("Vertices : %d\n", VERTICES);
    printf("Edges    : %d\n\n", EDGES);
    printf("Edge       Weight\n");
    printf("-----------------\n");

    for (i = 0; i < EDGES; i++)
        printf("(%d, %d)    %6d\n",
               edges[i].u, edges[i].v, edges[i].weight);
}

static void floyd_warshall(
    const int original[VERTICES][VERTICES],
    int distance[VERTICES][VERTICES],
    int next[VERTICES][VERTICES]
)
{
    int i, j, k;

    for (i = 0; i < VERTICES; i++) {
        for (j = 0; j < VERTICES; j++) {
            distance[i][j] = original[i][j];

            if (i != j && original[i][j] != INF)
                next[i][j] = j;
            else
                next[i][j] = -1;
        }
    }

    for (k = 0; k < VERTICES; k++) {
        for (i = 0; i < VERTICES; i++) {
            for (j = 0; j < VERTICES; j++) {
                if (distance[i][k] != INF &&
                    distance[k][j] != INF &&
                    distance[i][k] + distance[k][j] < distance[i][j]) {
                    distance[i][j] = distance[i][k] + distance[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
    }
}

static void print_path(
    int start,
    int end,
    const int next[VERTICES][VERTICES]
)
{
    int current;
    int safety = 0;

    if (next[start][end] == -1) {
        printf("Path     : No path\n");
        return;
    }

    printf("Path     : %d", start);
    current = start;

    while (current != end && safety < VERTICES) {
        current = next[current][end];
        printf(" -> %d", current);
        safety++;
    }

    printf("\n");
}

int main(void)
{
    Edge edges[EDGES];
    int matrix[VERTICES][VERTICES];
    int distance[VERTICES][VERTICES];
    int next[VERTICES][VERTICES];
    int u, v;

    srand((unsigned int)time(NULL));

    generate_connected_graph(edges, matrix);
    floyd_warshall(matrix, distance, next);

    print_graph(edges);

    printf("\nShortest Paths\n");
    printf("============================================================\n");

    for (u = 0; u < VERTICES; u++) {
        for (v = u + 1; v < VERTICES; v++) {
            printf("\n%d -> %d\n", u, v);
            print_path(u, v, next);
            printf("Distance : %d\n", distance[u][v]);
        }
    }

    printf("\nTotal vertex pairs : %d\n",
           VERTICES * (VERTICES - 1) / 2);

    return 0;
}
