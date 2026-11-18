#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KEY_MIN 0
#define KEY_MAX 1000000
#define SIZE_COUNT 6
#define OPS_DIVISOR 5
#define TIME_REPEATS 20
#define PHASE_COUNT 3

#ifndef SEED
#define SEED 20261118ULL
#endif

typedef enum {
    OP_INSERT,
    OP_DELETE
} OpType;

typedef struct {
    OpType type;
    int value;
} Operation;

typedef struct {
    int low;
    int high;
} Range;

typedef struct Node {
    int key;
    int count;
    int height;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct {
    unsigned long long comparisons;
    unsigned long long moves;
    unsigned long long rotations;
    size_t max_depth;
    size_t node_bytes;
    size_t peak_node_bytes;
} Stats;

typedef struct {
    unsigned long long comparisons;
    unsigned long long moves;
} Counter;

typedef struct {
    Counter phase[PHASE_COUNT];
    double ms[PHASE_COUNT];
    unsigned long long rotations;
    size_t memory;
    size_t max_depth;
    size_t final_nodes;
    int final_count;
    int height;
} Result;

static const char *phase_names[PHASE_COUNT] = {
    "Initial Sort",
    "Operations",
    "Final Output"
};

static unsigned long long rng_state = SEED;

static unsigned long long next_random(void)
{
    unsigned long long z;

    rng_state += 0x9E3779B97F4A7C15ULL;
    z = rng_state;
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

static int random_below(int bound)
{
    return (int)(next_random() % (unsigned long long)bound);
}

static int random_key(void)
{
    return KEY_MIN + random_below(KEY_MAX - KEY_MIN + 1);
}

static void *checked_malloc(size_t size)
{
    void *memory = malloc(size);

    if (memory == NULL)
    {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    return memory;
}

static int is_sorted(const int *a, int n)
{
    int i;

    for (i = 1; i < n; i++)
    {
        if (a[i - 1] > a[i])
            return 0;
    }
    return 1;
}

static void update_depth(Stats *s, size_t depth)
{
    if (depth > s->max_depth)
        s->max_depth = depth;
}

static int count_less(int x, int y, Stats *s)
{
    s->comparisons++;
    return x < y;
}

static void swap_values(int *a, int *b, Stats *s)
{
    int temp = *a;

    *a = *b;
    *b = temp;
    s->moves += 3;
}

static Counter snapshot(const Stats *s)
{
    Counter c;

    c.comparisons = s->comparisons;
    c.moves = s->moves;
    return c;
}

static Counter difference(Counter after, Counter before)
{
    Counter c;

    c.comparisons = after.comparisons - before.comparisons;
    c.moves = after.moves - before.moves;
    return c;
}

static void quick_sort_range(int *a, int low, int high, Stats *s, size_t depth)
{
    int pivot;
    int i;
    int j;

    update_depth(s, depth);
    if (low >= high)
        return;

    pivot = a[low + (high - low) / 2];
    i = low;
    j = high;

    while (i <= j)
    {
        while (count_less(a[i], pivot, s))
            i++;
        while (count_less(pivot, a[j], s))
            j--;
        if (i <= j)
        {
            if (i < j)
                swap_values(&a[i], &a[j], s);
            i++;
            j--;
        }
    }

    if (low < j)
        quick_sort_range(a, low, j, s, depth + 1);
    if (i < high)
        quick_sort_range(a, i, high, s, depth + 1);
}

static int lower_bound(const int *a, int len, int key, Stats *s)
{
    int low = 0;
    int high = len;

    while (low < high)
    {
        int mid = low + (high - low) / 2;

        if (count_less(a[mid], key, s))
            low = mid + 1;
        else
            high = mid;
    }
    return low;
}

static void array_insert(int *a, int *len, int key, Stats *s)
{
    int pos = lower_bound(a, *len, key, s);
    int j;

    for (j = *len; j > pos; j--)
    {
        a[j] = a[j - 1];
        s->moves++;
    }
    a[pos] = key;
    s->moves++;
    (*len)++;
}

static int array_delete(int *a, int *len, int key, Stats *s)
{
    int pos = lower_bound(a, *len, key, s);
    int j;

    if (pos == *len)
        return 0;
    s->comparisons++;
    if (a[pos] != key)
        return 0;

    for (j = pos; j < *len - 1; j++)
    {
        a[j] = a[j + 1];
        s->moves++;
    }
    (*len)--;
    return 1;
}

static void array_apply_operations(int *a, int *len, const Operation *ops,
                                   int op_count, Stats *s)
{
    int i;

    for (i = 0; i < op_count; i++)
    {
        if (ops[i].type == OP_INSERT)
            array_insert(a, len, ops[i].value, s);
        else if (!array_delete(a, len, ops[i].value, s))
        {
            fprintf(stderr, "Array delete failed: %d\n", ops[i].value);
            exit(EXIT_FAILURE);
        }
    }
}

static int height(const Node *n)
{
    return n != NULL ? n->height : 0;
}

static int max_int(int a, int b)
{
    return a > b ? a : b;
}

static void update_height(Node *n)
{
    n->height = 1 + max_int(height(n->left), height(n->right));
}

static Node *rotate_right(Node *y, Stats *s)
{
    Node *x = y->left;

    y->left = x->right;
    x->right = y;
    update_height(y);
    update_height(x);
    s->rotations++;
    return x;
}

static Node *rotate_left(Node *x, Stats *s)
{
    Node *y = x->right;

    x->right = y->left;
    y->left = x;
    update_height(x);
    update_height(y);
    s->rotations++;
    return y;
}

static Node *rebalance(Node *n, Stats *s)
{
    int balance;

    update_height(n);
    balance = height(n->left) - height(n->right);

    if (balance > 1)
    {
        if (height(n->left->left) < height(n->left->right))
            n->left = rotate_left(n->left, s);
        return rotate_right(n, s);
    }
    if (balance < -1)
    {
        if (height(n->right->right) < height(n->right->left))
            n->right = rotate_right(n->right, s);
        return rotate_left(n, s);
    }
    return n;
}

static Node *new_node(int key, Stats *s)
{
    Node *n = (Node *)checked_malloc(sizeof(Node));

    n->key = key;
    n->count = 1;
    n->height = 1;
    n->left = NULL;
    n->right = NULL;
    s->moves++;
    s->node_bytes += sizeof(Node);
    if (s->node_bytes > s->peak_node_bytes)
        s->peak_node_bytes = s->node_bytes;
    return n;
}

static void free_node(Node *n, Stats *s)
{
    s->node_bytes -= sizeof(Node);
    free(n);
}

static Node *avl_insert(Node *n, int key, Stats *s, size_t depth)
{
    update_depth(s, depth);
    if (n == NULL)
        return new_node(key, s);

    if (count_less(key, n->key, s))
        n->left = avl_insert(n->left, key, s, depth + 1);
    else if (count_less(n->key, key, s))
        n->right = avl_insert(n->right, key, s, depth + 1);
    else
    {
        n->count++;
        return n;
    }
    return rebalance(n, s);
}

static Node *detach_min(Node *n, Node **min_node, Stats *s, size_t depth)
{
    update_depth(s, depth);
    if (n->left == NULL)
    {
        *min_node = n;
        return n->right;
    }
    n->left = detach_min(n->left, min_node, s, depth + 1);
    return rebalance(n, s);
}

static Node *avl_delete(Node *n, int key, int *removed, Stats *s, size_t depth)
{
    update_depth(s, depth);
    if (n == NULL)
        return NULL;

    if (count_less(key, n->key, s))
        n->left = avl_delete(n->left, key, removed, s, depth + 1);
    else if (count_less(n->key, key, s))
        n->right = avl_delete(n->right, key, removed, s, depth + 1);
    else
    {
        *removed = 1;
        if (n->count > 1)
        {
            n->count--;
            return n;
        }
        if (n->left == NULL || n->right == NULL)
        {
            Node *child = n->left != NULL ? n->left : n->right;

            free_node(n, s);
            return child;
        }
        {
            Node *successor = NULL;

            n->right = detach_min(n->right, &successor, s, depth + 1);
            n->key = successor->key;
            n->count = successor->count;
            s->moves++;
            free_node(successor, s);
        }
    }
    return rebalance(n, s);
}

static void avl_inorder(const Node *n, int *out, int *idx, Stats *s, size_t depth)
{
    int k;

    update_depth(s, depth);
    if (n == NULL)
        return;

    avl_inorder(n->left, out, idx, s, depth + 1);
    for (k = 0; k < n->count; k++)
    {
        out[(*idx)++] = n->key;
        s->moves++;
    }
    avl_inorder(n->right, out, idx, s, depth + 1);
}

static void free_tree(Node *n)
{
    if (n == NULL)
        return;
    free_tree(n->left);
    free_tree(n->right);
    free(n);
}

static Node *avl_build(const int *src, int n, Stats *s)
{
    Node *root = NULL;
    int i;

    for (i = 0; i < n; i++)
        root = avl_insert(root, src[i], s, 1);
    return root;
}

static Node *avl_apply_operations(Node *root, int *count, const Operation *ops,
                                  int op_count, Stats *s)
{
    int i;

    for (i = 0; i < op_count; i++)
    {
        if (ops[i].type == OP_INSERT)
        {
            root = avl_insert(root, ops[i].value, s, 1);
            (*count)++;
        }
        else
        {
            int removed = 0;

            root = avl_delete(root, ops[i].value, &removed, s, 1);
            if (!removed)
            {
                fprintf(stderr, "AVL delete failed: %d\n", ops[i].value);
                exit(EXIT_FAILURE);
            }
            (*count)--;
        }
    }
    return root;
}

static int generate_operations(const int *src, int n, Operation *ops, int op_count,
                               int *pool, int *insert_count)
{
    int len = n;
    int i;

    memcpy(pool, src, (size_t)n * sizeof(int));
    *insert_count = 0;

    for (i = 0; i < op_count; i++)
    {
        if (random_below(2) == 0)
        {
            ops[i].type = OP_INSERT;
            ops[i].value = random_key();
            pool[len++] = ops[i].value;
            (*insert_count)++;
        }
        else
        {
            int idx = random_below(len);

            ops[i].type = OP_DELETE;
            ops[i].value = pool[idx];
            pool[idx] = pool[--len];
        }
    }
    return len;
}

static void run_array_counted(const int *src, int n, const Operation *ops,
                              int op_count, int capacity, int *initial,
                              int *final_out, Result *r)
{
    Stats s;
    Counter c0;
    Counter c1;
    int len = n;

    memset(&s, 0, sizeof(s));
    memcpy(final_out, src, (size_t)n * sizeof(int));

    c0 = snapshot(&s);
    if (n > 1)
        quick_sort_range(final_out, 0, n - 1, &s, 1);
    c1 = snapshot(&s);
    r->phase[0] = difference(c1, c0);
    memcpy(initial, final_out, (size_t)n * sizeof(int));

    array_apply_operations(final_out, &len, ops, op_count, &s);
    r->phase[1] = difference(snapshot(&s), c1);
    r->phase[2].comparisons = 0;
    r->phase[2].moves = 0;

    r->rotations = 0;
    r->max_depth = s.max_depth;
    r->memory = (size_t)capacity * sizeof(int) + s.max_depth * sizeof(Range);
    r->final_nodes = 0;
    r->final_count = len;
    r->height = 0;
}

static void run_avl_counted(const int *src, int n, const Operation *ops,
                            int op_count, int *initial, int *final_out, Result *r)
{
    Stats s;
    Counter c0;
    Counter c1;
    Counter c2;
    Node *root;
    int count = 0;
    int idx = 0;

    memset(&s, 0, sizeof(s));

    c0 = snapshot(&s);
    root = avl_build(src, n, &s);
    avl_inorder(root, initial, &idx, &s, 1);
    count = idx;
    c1 = snapshot(&s);
    r->phase[0] = difference(c1, c0);

    root = avl_apply_operations(root, &count, ops, op_count, &s);
    c2 = snapshot(&s);
    r->phase[1] = difference(c2, c1);

    idx = 0;
    avl_inorder(root, final_out, &idx, &s, 1);
    r->phase[2] = difference(snapshot(&s), c2);

    if (idx != count)
    {
        fprintf(stderr, "AVL count mismatch.\n");
        exit(EXIT_FAILURE);
    }

    r->rotations = s.rotations;
    r->max_depth = s.max_depth;
    r->memory = s.peak_node_bytes + s.max_depth * sizeof(Node *);
    r->final_nodes = s.node_bytes / sizeof(Node);
    r->final_count = count;
    r->height = height(root);
    free_tree(root);
}

static double elapsed_ms(clock_t start, clock_t end)
{
    return (double)(end - start) * 1000.0 / (double)CLOCKS_PER_SEC / TIME_REPEATS;
}

static void time_array(const int *src, int n, const Operation *ops, int op_count,
                       int capacity, double *ms)
{
    int *batch = (int *)checked_malloc((size_t)TIME_REPEATS * (size_t)capacity * sizeof(int));
    int lens[TIME_REPEATS];
    Stats s;
    clock_t start;
    clock_t end;
    int r;

    memset(&s, 0, sizeof(s));
    for (r = 0; r < TIME_REPEATS; r++)
    {
        memcpy(batch + (size_t)r * (size_t)capacity, src, (size_t)n * sizeof(int));
        lens[r] = n;
    }

    start = clock();
    for (r = 0; r < TIME_REPEATS; r++)
        quick_sort_range(batch + (size_t)r * (size_t)capacity, 0, n - 1, &s, 1);
    end = clock();
    ms[0] = elapsed_ms(start, end);

    start = clock();
    for (r = 0; r < TIME_REPEATS; r++)
        array_apply_operations(batch + (size_t)r * (size_t)capacity, &lens[r],
                               ops, op_count, &s);
    end = clock();
    ms[1] = elapsed_ms(start, end);
    ms[2] = 0.0;

    for (r = 0; r < TIME_REPEATS; r++)
    {
        if (!is_sorted(batch + (size_t)r * (size_t)capacity, lens[r]))
        {
            fprintf(stderr, "Array timing run failed.\n");
            exit(EXIT_FAILURE);
        }
    }
    free(batch);
}

static void time_avl(const int *src, int n, const Operation *ops, int op_count,
                     int capacity, double *ms)
{
    int *batch = (int *)checked_malloc((size_t)TIME_REPEATS * (size_t)capacity * sizeof(int));
    Node *roots[TIME_REPEATS];
    int counts[TIME_REPEATS];
    Stats s;
    clock_t start;
    clock_t end;
    int r;

    memset(&s, 0, sizeof(s));

    start = clock();
    for (r = 0; r < TIME_REPEATS; r++)
    {
        int idx = 0;

        roots[r] = avl_build(src, n, &s);
        avl_inorder(roots[r], batch + (size_t)r * (size_t)capacity, &idx, &s, 1);
        counts[r] = idx;
    }
    end = clock();
    ms[0] = elapsed_ms(start, end);

    start = clock();
    for (r = 0; r < TIME_REPEATS; r++)
        roots[r] = avl_apply_operations(roots[r], &counts[r], ops, op_count, &s);
    end = clock();
    ms[1] = elapsed_ms(start, end);

    start = clock();
    for (r = 0; r < TIME_REPEATS; r++)
    {
        int idx = 0;

        avl_inorder(roots[r], batch + (size_t)r * (size_t)capacity, &idx, &s, 1);
    }
    end = clock();
    ms[2] = elapsed_ms(start, end);

    for (r = 0; r < TIME_REPEATS; r++)
    {
        if (!is_sorted(batch + (size_t)r * (size_t)capacity, counts[r]))
        {
            fprintf(stderr, "AVL timing run failed.\n");
            exit(EXIT_FAILURE);
        }
        free_tree(roots[r]);
    }
    free(batch);
}

static double total_ms(const Result *r)
{
    return r->ms[0] + r->ms[1] + r->ms[2];
}

static unsigned long long total_comparisons(const Result *r)
{
    return r->phase[0].comparisons + r->phase[1].comparisons + r->phase[2].comparisons;
}

static unsigned long long total_moves(const Result *r)
{
    return r->phase[0].moves + r->phase[1].moves + r->phase[2].moves;
}

int main(void)
{
    const int sizes[SIZE_COUNT] = {1000, 2000, 4000, 8000, 16000, 20000};
    Result quick_results[SIZE_COUNT];
    Result avl_results[SIZE_COUNT];
    int insert_counts[SIZE_COUNT];
    int z;
    int p;

    printf("Quick Sort vs AVL Tree Sort (seed = %llu)\n", (unsigned long long)SEED);
    printf("Quick pivot: middle element (Hoare partition), sorted array updated with binary search\n");
    printf("Operations : n/%d random inserts/deletes, deletes always target an existing value\n",
           OPS_DIVISOR);
    printf("Time       : average of %d runs on the same data\n\n", TIME_REPEATS);
    printf("[1] Results\n");
    printf("========================================\n");

    for (z = 0; z < SIZE_COUNT; z++)
    {
        int n = sizes[z];
        int op_count = n / OPS_DIVISOR;
        int capacity = n + op_count;
        int *src = (int *)checked_malloc((size_t)n * sizeof(int));
        int *pool = (int *)checked_malloc((size_t)capacity * sizeof(int));
        int *quick_initial = (int *)checked_malloc((size_t)capacity * sizeof(int));
        int *quick_final = (int *)checked_malloc((size_t)capacity * sizeof(int));
        int *avl_initial = (int *)checked_malloc((size_t)capacity * sizeof(int));
        int *avl_final = (int *)checked_malloc((size_t)capacity * sizeof(int));
        Operation *ops = (Operation *)checked_malloc((size_t)op_count * sizeof(Operation));
        Result *q = &quick_results[z];
        Result *v = &avl_results[z];
        int expected_count;
        int i;
        int quick_ok;
        int avl_ok;

        for (i = 0; i < n; i++)
            src[i] = random_key();
        expected_count = generate_operations(src, n, ops, op_count, pool,
                                             &insert_counts[z]);

        run_array_counted(src, n, ops, op_count, capacity, quick_initial, quick_final, q);
        run_avl_counted(src, n, ops, op_count, avl_initial, avl_final, v);
        time_array(src, n, ops, op_count, capacity, q->ms);
        time_avl(src, n, ops, op_count, capacity, v->ms);

        quick_ok = is_sorted(quick_initial, n) && is_sorted(quick_final, q->final_count) &&
                   q->final_count == expected_count;
        avl_ok = is_sorted(avl_initial, n) && is_sorted(avl_final, v->final_count) &&
                 v->final_count == expected_count;
        if (memcmp(quick_initial, avl_initial, (size_t)n * sizeof(int)) != 0 ||
            q->final_count != v->final_count ||
            memcmp(quick_final, avl_final, (size_t)q->final_count * sizeof(int)) != 0)
        {
            quick_ok = 0;
            avl_ok = 0;
        }

        printf("\nData Size  : %d\n", n);
        printf("Operations : %d (Insert %d, Delete %d)\n\n", op_count,
               insert_counts[z], op_count - insert_counts[z]);
        printf("%-20s %16s %16s\n", "", "Quick Sort", "AVL Tree");
        printf("------------------------------------------------------\n");
        printf("%-20s %13.4f ms %13.4f ms\n", "Total Time", total_ms(q), total_ms(v));
        printf("%-20s %16llu %16llu\n", "Comparisons", total_comparisons(q), total_comparisons(v));
        printf("%-20s %16llu %16llu\n", "Data Movements", total_moves(q), total_moves(v));
        printf("%-20s %10lu Bytes %10lu Bytes\n", "Memory Usage",
               (unsigned long)q->memory, (unsigned long)v->memory);
        printf("%-20s %16d %16d\n", "Final Data Count", q->final_count, v->final_count);
        printf("%-20s %16s %16s\n", "Sorted Check", quick_ok ? "PASS" : "FAIL",
               avl_ok ? "PASS" : "FAIL");

        free(src);
        free(pool);
        free(quick_initial);
        free(quick_final);
        free(avl_initial);
        free(avl_final);
        free(ops);

        if (!quick_ok || !avl_ok)
            return EXIT_FAILURE;
    }

    printf("\n[2] Phase Breakdown\n");
    printf("========================================\n\n");
    printf("%6s | %-10s | %-12s | %10s | %12s | %12s\n",
           "Size", "Method", "Phase", "Time(ms)", "Comparisons", "Moves");
    printf("-------+------------+--------------+------------+--------------+-------------\n");
    for (z = 0; z < SIZE_COUNT; z++)
    {
        for (p = 0; p < PHASE_COUNT; p++)
        {
            printf("%6d | %-10s | %-12s | %10.4f | %12llu | %12llu\n", sizes[z],
                   "Quick", phase_names[p], quick_results[z].ms[p],
                   quick_results[z].phase[p].comparisons, quick_results[z].phase[p].moves);
        }
        for (p = 0; p < PHASE_COUNT; p++)
        {
            printf("%6d | %-10s | %-12s | %10.4f | %12llu | %12llu\n", sizes[z],
                   "AVL", phase_names[p], avl_results[z].ms[p],
                   avl_results[z].phase[p].comparisons, avl_results[z].phase[p].moves);
        }
    }

    printf("\n[3] Structure Details\n");
    printf("========================================\n\n");
    printf("%6s | %10s | %10s | %11s | %8s | %9s | %10s\n",
           "Size", "Final Data", "AVL Nodes", "AVL Height", "Rotation",
           "AVL Depth", "Quick Depth");
    printf("-------+------------+------------+-------------+----------+-----------+-----------\n");
    for (z = 0; z < SIZE_COUNT; z++)
    {
        printf("%6d | %10d | %10lu | %11d | %8llu | %9lu | %10lu\n", sizes[z],
               avl_results[z].final_count, (unsigned long)avl_results[z].final_nodes,
               avl_results[z].height, avl_results[z].rotations,
               (unsigned long)avl_results[z].max_depth,
               (unsigned long)quick_results[z].max_depth);
    }
    printf("\nNode size : %lu bytes, int size : %lu bytes\n",
           (unsigned long)sizeof(Node), (unsigned long)sizeof(int));
    return 0;
}