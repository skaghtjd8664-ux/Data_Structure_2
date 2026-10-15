#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define V 50
#define SPARSE_E 100
#define DENSE_E 600
#define MIN_WEIGHT 1
#define MAX_WEIGHT 1000

typedef struct {
    int u, v, weight;
} Edge;

typedef struct AdjNode {
    int vertex;
    int weight;
    struct AdjNode *next;
} AdjNode;

typedef struct {
    AdjNode *head[V];
} AdjList;

typedef struct {
    Edge edges[V - 1];
    int count;
    int total_cost;
} MSTResult;

static void allocation_error(void)
{
    fprintf(stderr, "Memory allocation failed.\n");
    exit(EXIT_FAILURE);
}

static int random_weight(void)
{
    return rand() % (MAX_WEIGHT - MIN_WEIGHT + 1) + MIN_WEIGHT;
}

static void init_matrix(int matrix[V][V])
{
    int i, j;
    for (i = 0; i < V; i++)
        for (j = 0; j < V; j++)
            matrix[i][j] = 0;
}

static void add_edge(Edge *edges, int *edge_count,
                     int matrix[V][V], int u, int v, int weight)
{
    edges[*edge_count].u = u;
    edges[*edge_count].v = v;
    edges[*edge_count].weight = weight;
    (*edge_count)++;
    matrix[u][v] = weight;
    matrix[v][u] = weight;
}

static void generate_graph(Edge *edges, int edge_count,
                           int matrix[V][V])
{
    int current = 0;
    int i;
    int u, v;

    init_matrix(matrix);

    /* 먼저 무작위 순서의 신장 트리를 생성하여 연결성을 보장한다. */
    for (i = 1; i < V; i++) {
        u = rand() % i;
        v = i;
        add_edge(edges, &current, matrix, u, v, random_weight());
    }

    while (current < edge_count) {
        u = rand() % V;
        v = rand() % V;

        if (u == v || matrix[u][v] != 0)
            continue;

        add_edge(edges, &current, matrix, u, v, random_weight());
    }
}

static void init_list(AdjList *graph)
{
    int i;
    for (i = 0; i < V; i++)
        graph->head[i] = NULL;
}

static void list_add(AdjList *graph, int u, int v, int weight)
{
    AdjNode *node = (AdjNode *)malloc(sizeof(AdjNode));
    if (node == NULL)
        allocation_error();

    node->vertex = v;
    node->weight = weight;
    node->next = graph->head[u];
    graph->head[u] = node;
}

static void build_list(AdjList *graph, const Edge *edges, int edge_count)
{
    int i;
    init_list(graph);

    for (i = 0; i < edge_count; i++) {
        list_add(graph, edges[i].u, edges[i].v, edges[i].weight);
        list_add(graph, edges[i].v, edges[i].u, edges[i].weight);
    }
}

static void free_list(AdjList *graph)
{
    int i;
    AdjNode *cur;
    AdjNode *next;

    for (i = 0; i < V; i++) {
        cur = graph->head[i];
        while (cur != NULL) {
            next = cur->next;
            free(cur);
            cur = next;
        }
        graph->head[i] = NULL;
    }
}

static int compare_edges(const void *a, const void *b)
{
    const Edge *x = (const Edge *)a;
    const Edge *y = (const Edge *)b;
    return x->weight - y->weight;
}

static void make_set(int parent[V], int rank[V])
{
    int i;
    for (i = 0; i < V; i++) {
        parent[i] = i;
        rank[i] = 0;
    }
}

static int find_set(int parent[V], int x)
{
    if (parent[x] != x)
        parent[x] = find_set(parent, parent[x]);
    return parent[x];
}

static void union_set(int parent[V], int rank[V], int a, int b)
{
    int root_a = find_set(parent, a);
    int root_b = find_set(parent, b);

    if (root_a == root_b)
        return;

    if (rank[root_a] < rank[root_b])
        parent[root_a] = root_b;
    else if (rank[root_a] > rank[root_b])
        parent[root_b] = root_a;
    else {
        parent[root_b] = root_a;
        rank[root_a]++;
    }
}

static MSTResult kruskal(const Edge *edges, int edge_count)
{
    Edge *sorted;
    int parent[V];
    int rank[V];
    int i;
    MSTResult result;

    result.count = 0;
    result.total_cost = 0;
    sorted = (Edge *)malloc(sizeof(Edge) * edge_count);
    if (sorted == NULL)
        allocation_error();

    for (i = 0; i < edge_count; i++)
        sorted[i] = edges[i];

    qsort(sorted, edge_count, sizeof(Edge), compare_edges);
    make_set(parent, rank);

    for (i = 0; i < edge_count && result.count < V - 1; i++) {
        if (find_set(parent, sorted[i].u) != find_set(parent, sorted[i].v)) {
            result.edges[result.count++] = sorted[i];
            result.total_cost += sorted[i].weight;
            union_set(parent, rank, sorted[i].u, sorted[i].v);
        }
    }

    free(sorted);
    return result;
}

static MSTResult prim(const int matrix[V][V])
{
    int selected[V] = {0};
    int min_cost[V];
    int parent[V];
    int i, j;
    int next;
    MSTResult result;

    result.count = 0;
    result.total_cost = 0;

    for (i = 0; i < V; i++) {
        min_cost[i] = 2147483647;
        parent[i] = -1;
    }

    min_cost[0] = 0;

    for (i = 0; i < V; i++) {
        next = -1;

        for (j = 0; j < V; j++) {
            if (!selected[j] && (next == -1 || min_cost[j] < min_cost[next]))
                next = j;
        }

        if (next == -1 || min_cost[next] == 2147483647)
            break;

        selected[next] = 1;

        if (parent[next] != -1) {
            Edge chosen;
            chosen.u = parent[next];
            chosen.v = next;
            chosen.weight = matrix[parent[next]][next];
            result.edges[result.count++] = chosen;
            result.total_cost += chosen.weight;
        }

        for (j = 0; j < V; j++) {
            if (!selected[j] && matrix[next][j] != 0 &&
                matrix[next][j] < min_cost[j]) {
                min_cost[j] = matrix[next][j];
                parent[j] = next;
            }
        }
    }

    return result;
}

static long list_memory(const AdjList *graph, int edge_count)
{
    (void)graph;
    return (long)sizeof(AdjList) + (long)(2 * edge_count) * sizeof(AdjNode) +
           (long)edge_count * sizeof(Edge) +
           (long)V * (sizeof(int) + sizeof(int));
}

static long matrix_memory(int edge_count)
{
    return (long)sizeof(int) * V * V +
           (long)V * sizeof(int) * 3 +
           (long)(V - 1) * sizeof(Edge) +
           (long)edge_count * sizeof(Edge);
}

static double elapsed_ms(clock_t start, clock_t end)
{
    return (double)(end - start) * 1000.0 / (double)CLOCKS_PER_SEC;
}

static void print_result(const char *title, const MSTResult *result,
                         int edge_count, long memory, double time_ms)
{
    int i;

    printf("\n============================================================\n");
    printf("%s\n", title);
    printf("============================================================\n");
    printf("Vertices       : %d\n", V);
    printf("Edges          : %d\n", edge_count);
    printf("MST Edges      : %d\n", result->count);
    printf("MST Cost       : %d\n", result->total_cost);
    printf("Memory         : %ld Bytes\n", memory);
    printf("Execution Time : %.6f ms\n", time_ms);
    printf("MST edges:\n");

    for (i = 0; i < result->count; i++)
        printf("(%d, %d, weight=%d)\n", result->edges[i].u,
               result->edges[i].v, result->edges[i].weight);
}

int main(void)
{
    Edge sparse_edges[SPARSE_E];
    Edge dense_edges[DENSE_E];
    int sparse_matrix[V][V];
    int dense_matrix[V][V];
    AdjList sparse_list;
    AdjList dense_list;
    MSTResult result;
    clock_t start;
    clock_t end;

    srand((unsigned int)time(NULL));

    generate_graph(sparse_edges, SPARSE_E, sparse_matrix);
    generate_graph(dense_edges, DENSE_E, dense_matrix);
    build_list(&sparse_list, sparse_edges, SPARSE_E);
    build_list(&dense_list, dense_edges, DENSE_E);

    start = clock();
    result = kruskal(sparse_edges, SPARSE_E);
    end = clock();
    print_result("Case 1: Sparse Graph - Adjacency List - Kruskal",
                 &result, SPARSE_E, list_memory(&sparse_list, SPARSE_E),
                 elapsed_ms(start, end));

    start = clock();
    result = kruskal(dense_edges, DENSE_E);
    end = clock();
    print_result("Case 2: Dense Graph - Adjacency List - Kruskal",
                 &result, DENSE_E, list_memory(&dense_list, DENSE_E),
                 elapsed_ms(start, end));

    start = clock();
    result = prim(sparse_matrix);
    end = clock();
    print_result("Case 3: Sparse Graph - Adjacency Matrix - Prim",
                 &result, SPARSE_E, matrix_memory(SPARSE_E),
                 elapsed_ms(start, end));

    start = clock();
    result = prim(dense_matrix);
    end = clock();
    print_result("Case 4: Dense Graph - Adjacency Matrix - Prim",
                 &result, DENSE_E, matrix_memory(DENSE_E),
                 elapsed_ms(start, end));

    free_list(&sparse_list);
    free_list(&dense_list);
    return 0;
}
