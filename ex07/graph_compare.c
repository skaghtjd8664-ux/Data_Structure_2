#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define VERTICES 100
#define SPARSE_EDGES 100
#define DENSE_EDGES 4000
#define TESTS 4

typedef struct AdjNode {
    int vertex;
    struct AdjNode *next;
} AdjNode;

typedef struct {
    AdjNode *head[VERTICES];
} AdjList;

typedef struct {
    int u;
    int v;
} Edge;

typedef struct {
    long insertion;
    long deletion;
    long connectivity;
    long adjacent;
} Counts;

static void die(void)
{
    fprintf(stderr, "Memory allocation failed.\n");
    exit(EXIT_FAILURE);
}

static int random_vertex(void)
{
    return rand() % VERTICES;
}

static void create_random_graph(int matrix[VERTICES][VERTICES], Edge *edges, int edge_count)
{
    int i;
    int u;
    int v;

    for (i = 0; i < VERTICES; i++)
        for (v = 0; v < VERTICES; v++)
            matrix[i][v] = 0;

    i = 0;
    while (i < edge_count) {
        u = random_vertex();
        v = random_vertex();

        if (u == v || matrix[u][v] == 1)
            continue;

        matrix[u][v] = 1;
        matrix[v][u] = 1;
        edges[i].u = u;
        edges[i].v = v;
        i++;
    }
}

static void matrix_copy(int dest[VERTICES][VERTICES], const int src[VERTICES][VERTICES])
{
    int i;
    int j;

    for (i = 0; i < VERTICES; i++)
        for (j = 0; j < VERTICES; j++)
            dest[i][j] = src[i][j];
}

static void list_init(AdjList *graph)
{
    int i;

    for (i = 0; i < VERTICES; i++)
        graph->head[i] = NULL;
}

static AdjNode *new_node(int vertex)
{
    AdjNode *node = (AdjNode *)malloc(sizeof(AdjNode));

    if (node == NULL)
        die();

    node->vertex = vertex;
    node->next = NULL;
    return node;
}

static void list_insert_initial(AdjList *graph, int u, int v)
{
    AdjNode *node_u = new_node(v);
    AdjNode *node_v = new_node(u);

    node_u->next = graph->head[u];
    graph->head[u] = node_u;

    node_v->next = graph->head[v];
    graph->head[v] = node_v;
}

static void build_list_from_matrix(AdjList *graph, const int matrix[VERTICES][VERTICES])
{
    int u;
    int v;

    list_init(graph);

    for (u = 0; u < VERTICES; u++)
        for (v = u + 1; v < VERTICES; v++)
            if (matrix[u][v])
                list_insert_initial(graph, u, v);
}

static long list_memory(const AdjList *graph)
{
    long bytes = (long)sizeof(AdjList);
    int u;
    AdjNode *cur;

    for (u = 0; u < VERTICES; u++) {
        cur = graph->head[u];
        while (cur != NULL) {
            bytes += (long)sizeof(AdjNode);
            cur = cur->next;
        }
    }

    return bytes;
}

static void list_free(AdjList *graph)
{
    int u;
    AdjNode *cur;
    AdjNode *next;

    for (u = 0; u < VERTICES; u++) {
        cur = graph->head[u];
        while (cur != NULL) {
            next = cur->next;
            free(cur);
            cur = next;
        }
        graph->head[u] = NULL;
    }
}

static long matrix_memory(void)
{
    return (long)sizeof(int) * VERTICES * VERTICES;
}

static long matrix_insert(int matrix[VERTICES][VERTICES], int u, int v)
{
    long comparisons = 1;

    if (matrix[u][v] == 0) {
        matrix[u][v] = 1;
        matrix[v][u] = 1;
    }

    return comparisons;
}

static long matrix_delete(int matrix[VERTICES][VERTICES], int u, int v)
{
    long comparisons = 1;

    if (matrix[u][v] == 1) {
        matrix[u][v] = 0;
        matrix[v][u] = 0;
    }

    return comparisons;
}

static long matrix_connected(const int matrix[VERTICES][VERTICES], int u, int v, int *result)
{
    *result = matrix[u][v];
    return 1;
}

static long matrix_adjacent(const int matrix[VERTICES][VERTICES], int u)
{
    int v;
    long comparisons = 0;

    printf("  Adjacent vertices: ");
    for (v = 0; v < VERTICES; v++) {
        comparisons++;
        if (matrix[u][v])
            printf("%d ", v);
    }
    printf("\n");

    return comparisons;
}

static long list_find(const AdjList *graph, int u, int v, AdjNode **previous, AdjNode **found)
{
    AdjNode *prev = NULL;
    AdjNode *cur = graph->head[u];
    long comparisons = 0;

    while (cur != NULL) {
        comparisons++;
        if (cur->vertex == v)
            break;
        prev = cur;
        cur = cur->next;
    }

    if (previous != NULL)
        *previous = prev;
    if (found != NULL)
        *found = cur;

    return comparisons;
}

static long list_insert(AdjList *graph, int u, int v)
{
    AdjNode *found;
    long comparisons = list_find(graph, u, v, NULL, &found);

    if (found == NULL) {
        AdjNode *node_u = new_node(v);
        AdjNode *node_v = new_node(u);
        node_u->next = graph->head[u];
        graph->head[u] = node_u;
        node_v->next = graph->head[v];
        graph->head[v] = node_v;
    }

    return comparisons;
}

static long list_delete(AdjList *graph, int u, int v)
{
    AdjNode *prev;
    AdjNode *found;
    long comparisons = list_find(graph, u, v, &prev, &found);

    if (found != NULL) {
        if (prev == NULL)
            graph->head[u] = found->next;
        else
            prev->next = found->next;
        free(found);

        comparisons += list_find(graph, v, u, &prev, &found);
        if (found != NULL) {
            if (prev == NULL)
                graph->head[v] = found->next;
            else
                prev->next = found->next;
            free(found);
        }
    }

    return comparisons;
}

static long list_connected(const AdjList *graph, int u, int v, int *result)
{
    AdjNode *found;
    long comparisons = list_find(graph, u, v, NULL, &found);
    *result = (found != NULL);
    return comparisons;
}

static long list_adjacent(const AdjList *graph, int u)
{
    AdjNode *cur = graph->head[u];
    long comparisons = 0;

    printf("  Adjacent vertices: ");
    while (cur != NULL) {
        comparisons++;
        printf("%d ", cur->vertex);
        cur = cur->next;
    }
    printf("\n");

    return comparisons;
}

static void choose_operations(const int matrix[VERTICES][VERTICES],
                              int *insert_u, int *insert_v,
                              int *delete_u, int *delete_v,
                              int *check_u, int *check_v,
                              int *adjacent_vertex)
{
    int u;
    int v;

    *delete_u = 0;
    *delete_v = 0;
    for (u = 0; u < VERTICES && *delete_u == *delete_v; u++)
        for (v = u + 1; v < VERTICES; v++)
            if (matrix[u][v]) {
                *delete_u = u;
                *delete_v = v;
                break;
            }

    *insert_u = 0;
    *insert_v = 0;
    for (u = 0; u < VERTICES && *insert_u == *insert_v; u++)
        for (v = u + 1; v < VERTICES; v++)
            if (!matrix[u][v]) {
                *insert_u = u;
                *insert_v = v;
                break;
            }

    *check_u = 0;
    *check_v = VERTICES - 1;
    if (*check_u == *check_v)
        *check_v = 1;

    *adjacent_vertex = 0;
}

static void print_case_header(const char *name)
{
    printf("\n============================================================\n");
    printf("%s\n", name);
    printf("============================================================\n");
}

static void run_matrix_case(const char *name, const int original[VERTICES][VERTICES],
                            int insert_u, int insert_v, int delete_u, int delete_v,
                            int check_u, int check_v, int adjacent_vertex)
{
    int matrix[VERTICES][VERTICES];
    int connected;
    Counts counts;

    matrix_copy(matrix, original);
    counts.insertion = matrix_insert(matrix, insert_u, insert_v);
    counts.deletion = matrix_delete(matrix, delete_u, delete_v);
    counts.connectivity = matrix_connected(matrix, check_u, check_v, &connected);

    print_case_header(name);
    printf("Memory                         : %ld Bytes\n", matrix_memory());
    printf("Edge insertion comparisons     : %ld\n", counts.insertion);
    printf("Edge deletion comparisons      : %ld\n", counts.deletion);
    printf("Connectivity check comparisons : %ld\n", counts.connectivity);
    counts.adjacent = matrix_adjacent(matrix, adjacent_vertex);
    printf("Adjacent vertices comparisons  : %ld\n", counts.adjacent);
    printf("Connectivity (%d, %d)           : %s\n", check_u, check_v,
           connected ? "Connected" : "Not connected");
}

static void run_list_case(const char *name, const int original[VERTICES][VERTICES],
                          int insert_u, int insert_v, int delete_u, int delete_v,
                          int check_u, int check_v, int adjacent_vertex)
{
    AdjList graph;
    int connected;
    Counts counts;

    build_list_from_matrix(&graph, original);
    counts.insertion = list_insert(&graph, insert_u, insert_v);
    counts.deletion = list_delete(&graph, delete_u, delete_v);
    counts.connectivity = list_connected(&graph, check_u, check_v, &connected);

    print_case_header(name);
    printf("Memory                         : %ld Bytes\n", list_memory(&graph));
    printf("Edge insertion comparisons     : %ld\n", counts.insertion);
    printf("Edge deletion comparisons      : %ld\n", counts.deletion);
    printf("Connectivity check comparisons : %ld\n", counts.connectivity);
    counts.adjacent = list_adjacent(&graph, adjacent_vertex);
    printf("Adjacent vertices comparisons  : %ld\n", counts.adjacent);
    printf("Connectivity (%d, %d)           : %s\n", check_u, check_v,
           connected ? "Connected" : "Not connected");

    list_free(&graph);
}

static void print_generated_edges(const char *name, const Edge *edges, int edge_count)
{
    int i;

    printf("\n%s: %d edges\n", name, edge_count);
    printf("Edges: ");
    for (i = 0; i < edge_count; i++) {
        printf("(%d,%d)", edges[i].u, edges[i].v);
        if (i + 1 < edge_count)
            printf(" ");
    }
    printf("\n");
}

int main(void)
{
    int sparse_matrix[VERTICES][VERTICES];
    int dense_matrix[VERTICES][VERTICES];
    Edge sparse_edges[SPARSE_EDGES];
    Edge dense_edges[DENSE_EDGES];
    int insert_u;
    int insert_v;
    int delete_u;
    int delete_v;
    int check_u;
    int check_v;
    int adjacent_vertex;

    srand((unsigned int)time(NULL));

    create_random_graph(sparse_matrix, sparse_edges, SPARSE_EDGES);
    create_random_graph(dense_matrix, dense_edges, DENSE_EDGES);

    print_generated_edges("Sparse Graph", sparse_edges, SPARSE_EDGES);
    print_generated_edges("Dense Graph", dense_edges, DENSE_EDGES);

    choose_operations(sparse_matrix, &insert_u, &insert_v, &delete_u, &delete_v,
                     &check_u, &check_v, &adjacent_vertex);
    printf("\nSparse operation pairs: insert=(%d,%d), delete=(%d,%d), check=(%d,%d), vertex=%d\n",
           insert_u, insert_v, delete_u, delete_v, check_u, check_v, adjacent_vertex);

    run_matrix_case("Case 1: Sparse Graph - Adjacency Matrix", sparse_matrix,
                    insert_u, insert_v, delete_u, delete_v,
                    check_u, check_v, adjacent_vertex);
    run_list_case("Case 2: Sparse Graph - Adjacency List", sparse_matrix,
                  insert_u, insert_v, delete_u, delete_v,
                  check_u, check_v, adjacent_vertex);

    choose_operations(dense_matrix, &insert_u, &insert_v, &delete_u, &delete_v,
                     &check_u, &check_v, &adjacent_vertex);
    printf("\nDense operation pairs: insert=(%d,%d), delete=(%d,%d), check=(%d,%d), vertex=%d\n",
           insert_u, insert_v, delete_u, delete_v, check_u, check_v, adjacent_vertex);

    run_matrix_case("Case 3: Dense Graph - Adjacency Matrix", dense_matrix,
                    insert_u, insert_v, delete_u, delete_v,
                    check_u, check_v, adjacent_vertex);
    run_list_case("Case 4: Dense Graph - Adjacency List", dense_matrix,
                  insert_u, insert_v, delete_u, delete_v,
                  check_u, check_v, adjacent_vertex);

    return 0;
}
