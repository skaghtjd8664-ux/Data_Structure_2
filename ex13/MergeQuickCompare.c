#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KEY_MIN 0
#define KEY_MAX 1000000
#define SIZE_COUNT 6
#define ALGO_COUNT 4
#define TIME_REPEATS 50
#define AVERAGE_TRIALS 100
#define SORTED_SIZE_COUNT 3

#ifndef SEED
#define SEED 20261108ULL
#endif

typedef enum {
    ALGO_MERGE,
    ALGO_QUICK_FIRST,
    ALGO_QUICK_RANDOM,
    ALGO_QUICK_MEDIAN3
} Algorithm;

typedef struct {
    int low;
    int high;
} Range;

typedef struct {
    unsigned long long comparisons;
    unsigned long long moves;
    unsigned long long calls;
    size_t array_memory;
    size_t max_depth;
} Metrics;

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

static void generate_data(int *a, int n)
{
    int i;

    for (i = 0; i < n; i++)
        a[i] = KEY_MIN + random_below(KEY_MAX - KEY_MIN + 1);
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

static int compare_int(const void *x, const void *y)
{
    int a = *(const int *)x;
    int b = *(const int *)y;

    return (a > b) - (a < b);
}

static size_t extra_memory(Metrics m)
{
    return m.array_memory + m.max_depth * sizeof(Range);
}

static void enter_call(Metrics *m, size_t depth)
{
    m->calls++;
    if (depth > m->max_depth)
        m->max_depth = depth;
}

static void swap_int(int *a, int *b, Metrics *m)
{
    int temp = *a;

    *a = *b;
    *b = temp;
    m->moves += 3;
}

static void merge_sort_range(int *a, int *temp, int left, int right,
                             Metrics *m, size_t depth)
{
    int mid;
    int i;
    int j;
    int k;

    enter_call(m, depth);
    if (left >= right)
        return;

    mid = left + (right - left) / 2;
    merge_sort_range(a, temp, left, mid, m, depth + 1);
    merge_sort_range(a, temp, mid + 1, right, m, depth + 1);

    i = left;
    j = mid + 1;
    k = left;
    while (i <= mid && j <= right)
    {
        m->comparisons++;
        if (a[i] <= a[j])
            temp[k++] = a[i++];
        else
            temp[k++] = a[j++];
        m->moves++;
    }
    while (i <= mid)
    {
        temp[k++] = a[i++];
        m->moves++;
    }
    while (j <= right)
    {
        temp[k++] = a[j++];
        m->moves++;
    }
    for (i = left; i <= right; i++)
    {
        a[i] = temp[i];
        m->moves++;
    }
}

static void merge_sort(int *a, int n, Metrics *m)
{
    int *temp;

    if (n < 2)
        return;

    temp = (int *)checked_malloc((size_t)n * sizeof(int));
    merge_sort_range(a, temp, 0, n - 1, m, 1);
    m->array_memory = (size_t)n * sizeof(int);
    free(temp);
}

static int median_of_three(const int *a, int x, int y, int z, Metrics *m)
{
    m->comparisons++;
    if (a[x] < a[y])
    {
        m->comparisons++;
        if (a[y] < a[z])
            return y;
        m->comparisons++;
        return (a[x] < a[z]) ? z : x;
    }

    m->comparisons++;
    if (a[x] < a[z])
        return x;
    m->comparisons++;
    return (a[y] < a[z]) ? z : y;
}

static int choose_pivot(const int *a, int low, int high, Algorithm algo,
                        Metrics *m)
{
    if (algo == ALGO_QUICK_FIRST)
        return low;
    if (algo == ALGO_QUICK_RANDOM)
        return low + random_below(high - low + 1);
    return median_of_three(a, low, low + (high - low) / 2, high, m);
}

static int partition_array(int *a, int low, int high, Algorithm algo,
                           Metrics *m)
{
    int pivot_index = choose_pivot(a, low, high, algo, m);
    int pivot;
    int i;
    int j;

    swap_int(&a[pivot_index], &a[high], m);
    pivot = a[high];
    i = low;

    for (j = low; j < high; j++)
    {
        m->comparisons++;
        if (a[j] <= pivot)
        {
            swap_int(&a[i], &a[j], m);
            i++;
        }
    }

    swap_int(&a[i], &a[high], m);
    return i;
}

static void quick_sort_range(int *a, int low, int high, Algorithm algo,
                             Metrics *m, size_t depth)
{
    int pivot;

    enter_call(m, depth);
    if (low >= high)
        return;

    pivot = partition_array(a, low, high, algo, m);
    quick_sort_range(a, low, pivot - 1, algo, m, depth + 1);
    quick_sort_range(a, pivot + 1, high, algo, m, depth + 1);
}

static void run_sort(Algorithm algo, int *a, int n, Metrics *m)
{
    if (algo == ALGO_MERGE)
        merge_sort(a, n, m);
    else if (n > 1)
        quick_sort_range(a, 0, n - 1, algo, m, 1);
}

static const char *algo_name(Algorithm algo)
{
    if (algo == ALGO_MERGE)
        return "Merge Sort";
    if (algo == ALGO_QUICK_FIRST)
        return "Quick (First)";
    if (algo == ALGO_QUICK_RANDOM)
        return "Quick (Random)";
    return "Quick (Median3)";
}

static Metrics sort_copy_and_check(Algorithm algo, const int *original,
                                   int *work, const int *reference, int n)
{
    Metrics m = {0, 0, 0, 0, 0};

    memcpy(work, original, (size_t)n * sizeof(int));
    run_sort(algo, work, n, &m);

    if (!is_sorted(work, n) ||
        (reference != NULL &&
         memcmp(work, reference, (size_t)n * sizeof(int)) != 0))
    {
        fprintf(stderr, "Sorting failed: %s, n = %d\n", algo_name(algo), n);
        exit(EXIT_FAILURE);
    }
    return m;
}

static double measure_ms(Algorithm algo, const int *original, int *batch, int n)
{
    clock_t start;
    clock_t end;
    int r;

    for (r = 0; r < TIME_REPEATS; r++)
        memcpy(batch + (size_t)r * (size_t)n, original, (size_t)n * sizeof(int));

    start = clock();
    for (r = 0; r < TIME_REPEATS; r++)
    {
        Metrics m = {0, 0, 0, 0, 0};
        run_sort(algo, batch + (size_t)r * (size_t)n, n, &m);
    }
    end = clock();

    for (r = 0; r < TIME_REPEATS; r++)
    {
        if (!is_sorted(batch + (size_t)r * (size_t)n, n))
        {
            fprintf(stderr, "Timing sort failed: %s\n", algo_name(algo));
            exit(EXIT_FAILURE);
        }
    }

    return (double)(end - start) * 1000.0 / (double)CLOCKS_PER_SEC / TIME_REPEATS;
}

static void print_table_header(void)
{
    printf("%-16s %-14s %-12s %-14s %s\n",
           "Algorithm", "Comparisons", "Time(ms)", "Memory(Bytes)", "Sorted");
    printf("---------------------------------------------------------------------\n");
}

static void run_main_experiment(const int *sizes)
{
    int s;
    int k;

    printf("[1] Random Data (Time = average of %d sorts on the same data)\n",
           TIME_REPEATS);
    printf("========================================\n");

    for (s = 0; s < SIZE_COUNT; s++)
    {
        int n = sizes[s];
        int *original = (int *)checked_malloc((size_t)n * sizeof(int));
        int *work = (int *)checked_malloc((size_t)n * sizeof(int));
        int *reference = (int *)checked_malloc((size_t)n * sizeof(int));
        int *batch = (int *)checked_malloc((size_t)TIME_REPEATS * (size_t)n * sizeof(int));

        generate_data(original, n);

        printf("\nData Size : %d\n", n);
        print_table_header();

        for (k = 0; k < ALGO_COUNT; k++)
        {
            Algorithm algo = (Algorithm)k;
            Metrics m = sort_copy_and_check(algo, original, work,
                                            k == 0 ? NULL : reference, n);
            double ms;

            if (k == 0)
                memcpy(reference, work, (size_t)n * sizeof(int));
            ms = measure_ms(algo, original, batch, n);

            printf("%-16s %-14llu %-12.4f %-14lu %s\n",
                   algo_name(algo), m.comparisons, ms,
                   (unsigned long)extra_memory(m), "PASS");
        }

        free(original);
        free(work);
        free(reference);
        free(batch);
    }
    printf("\n");
}

static void run_average_experiment(const int *sizes)
{
    Metrics sum[SIZE_COUNT][ALGO_COUNT];
    int s;
    int k;
    int trial;

    memset(sum, 0, sizeof(sum));

    for (s = 0; s < SIZE_COUNT; s++)
    {
        int n = sizes[s];
        int *original = (int *)checked_malloc((size_t)n * sizeof(int));
        int *work = (int *)checked_malloc((size_t)n * sizeof(int));
        int *reference = (int *)checked_malloc((size_t)n * sizeof(int));

        for (trial = 0; trial < AVERAGE_TRIALS; trial++)
        {
            generate_data(original, n);
            for (k = 0; k < ALGO_COUNT; k++)
            {
                Metrics m = sort_copy_and_check((Algorithm)k, original, work,
                                                k == 0 ? NULL : reference, n);

                if (k == 0)
                    memcpy(reference, work, (size_t)n * sizeof(int));
                sum[s][k].comparisons += m.comparisons;
                sum[s][k].moves += m.moves;
                sum[s][k].calls += m.calls;
                sum[s][k].array_memory += m.array_memory;
                sum[s][k].max_depth += m.max_depth;
            }
        }

        free(original);
        free(work);
        free(reference);
    }

    printf("[2] Random Data - Average of %d Trials\n", AVERAGE_TRIALS);
    printf("========================================\n\n");
    printf("%6s | %-15s | %11s | %11s | %8s | %8s | %13s\n",
           "Size", "Algorithm", "Comparisons", "Moves", "Calls", "MaxDepth",
           "Memory(Bytes)");
    printf("-------+-----------------+-------------+-------------+----------+----------+--------------\n");

    for (s = 0; s < SIZE_COUNT; s++)
    {
        for (k = 0; k < ALGO_COUNT; k++)
        {
            Metrics t = sum[s][k];
            double depth = (double)t.max_depth / AVERAGE_TRIALS;
            double memory = (double)t.array_memory / AVERAGE_TRIALS +
                            depth * (double)sizeof(Range);

            printf("%6d | %-15s | %11.1f | %11.1f | %8.1f | %8.1f | %13.1f\n",
                   sizes[s], algo_name((Algorithm)k),
                   (double)t.comparisons / AVERAGE_TRIALS,
                   (double)t.moves / AVERAGE_TRIALS,
                   (double)t.calls / AVERAGE_TRIALS,
                   depth, memory);
        }
    }
    printf("\nSorted Check : PASS (all sizes, all algorithms, %d trials)\n\n",
           AVERAGE_TRIALS);
}

static void run_sorted_experiment(void)
{
    const int sizes[SORTED_SIZE_COUNT] = {1000, 2000, 4000};
    int s;
    int k;

    printf("[3] Additional: Already Sorted Input\n");
    printf("========================================\n\n");
    printf("%6s | %-15s | %11s | %10s | %8s | %13s\n",
           "Size", "Algorithm", "Comparisons", "Time(ms)", "MaxDepth",
           "Memory(Bytes)");
    printf("-------+-----------------+-------------+------------+----------+--------------\n");

    for (s = 0; s < SORTED_SIZE_COUNT; s++)
    {
        int n = sizes[s];
        int *original = (int *)checked_malloc((size_t)n * sizeof(int));
        int *work = (int *)checked_malloc((size_t)n * sizeof(int));
        int *batch = (int *)checked_malloc((size_t)TIME_REPEATS * (size_t)n * sizeof(int));

        generate_data(original, n);
        qsort(original, (size_t)n, sizeof(int), compare_int);

        for (k = 0; k < ALGO_COUNT; k++)
        {
            Algorithm algo = (Algorithm)k;
            Metrics m = sort_copy_and_check(algo, original, work, NULL, n);
            double ms = measure_ms(algo, original, batch, n);

            printf("%6d | %-15s | %11llu | %10.4f | %8lu | %13lu\n",
                   n, algo_name(algo), m.comparisons, ms,
                   (unsigned long)m.max_depth, (unsigned long)extra_memory(m));
        }

        free(original);
        free(work);
        free(batch);
    }
}

int main(void)
{
    const int sizes[SIZE_COUNT] = {1000, 2000, 4000, 8000, 16000, 20000};

    printf("Merge Sort and Quick Sort Comparison (seed = %llu)\n",
           (unsigned long long)SEED);
    printf("Memory = auxiliary array + max recursion depth x %lu bytes (low, high)\n\n",
           (unsigned long)sizeof(Range));

    run_main_experiment(sizes);
    run_average_experiment(sizes);
    run_sorted_experiment();
    return 0;
}