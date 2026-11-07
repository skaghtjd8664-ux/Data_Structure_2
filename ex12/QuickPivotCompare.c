#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KEY_MIN 0
#define KEY_MAX 1000000
#define SIZE_COUNT 6
#define METHOD_COUNT 3
#define AVERAGE_TRIALS 100
#define SORTED_SIZE_COUNT 3
#define BONUS_SIZE 10000
#define BONUS_REPEATS 100
#define BONUS_ROUNDS 2
#define STACK_CAPACITY 64

#ifndef SEED
#define SEED 20261105ULL
#endif

typedef enum {
    PIVOT_FIRST,
    PIVOT_RANDOM,
    PIVOT_MEDIAN3
} PivotMethod;

typedef struct {
    unsigned long long partition_comparisons;
    unsigned long long pivot_comparisons;
    unsigned long long partitions;
} Metrics;

typedef struct {
    int low;
    int high;
} Range;

typedef void (*SortFunction)(int *data, int n, PivotMethod method, Metrics *metrics);

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

static unsigned long long total_comparisons(Metrics metrics)
{
    return metrics.partition_comparisons + metrics.pivot_comparisons;
}

static void add_metrics(Metrics *sum, Metrics value)
{
    sum->partition_comparisons += value.partition_comparisons;
    sum->pivot_comparisons += value.pivot_comparisons;
    sum->partitions += value.partitions;
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

static void generate_data(int *data, int n)
{
    int i;

    for (i = 0; i < n; i++)
        data[i] = KEY_MIN + random_below(KEY_MAX - KEY_MIN + 1);
}

static void swap_int(int *a, int *b)
{
    int temp = *a;

    *a = *b;
    *b = temp;
}

static int is_sorted(const int *data, int n)
{
    int i;

    for (i = 1; i < n; i++)
    {
        if (data[i - 1] > data[i])
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

static int choose_pivot_index(
    const int *data,
    int low,
    int high,
    PivotMethod method,
    Metrics *metrics
)
{
    int first;
    int middle;
    int last;

    if (method == PIVOT_FIRST)
        return low;

    if (method == PIVOT_RANDOM)
        return low + random_below(high - low + 1);

    first = low;
    middle = low + (high - low) / 2;
    last = high;

    metrics->pivot_comparisons++;
    if (data[first] < data[middle])
    {
        metrics->pivot_comparisons++;
        if (data[middle] < data[last])
            return middle;

        metrics->pivot_comparisons++;
        if (data[first] < data[last])
            return last;
        return first;
    }

    metrics->pivot_comparisons++;
    if (data[first] < data[last])
        return first;

    metrics->pivot_comparisons++;
    if (data[middle] < data[last])
        return last;
    return middle;
}

static int partition(
    int *data,
    int low,
    int high,
    PivotMethod method,
    Metrics *metrics
)
{
    int pivot_index;
    int pivot_value;
    int i;
    int j;

    metrics->partitions++;
    pivot_index = choose_pivot_index(data, low, high, method, metrics);
    pivot_value = data[pivot_index];
    swap_int(&data[pivot_index], &data[high]);

    i = low;
    for (j = low; j < high; j++)
    {
        metrics->partition_comparisons++;
        if (data[j] <= pivot_value)
        {
            swap_int(&data[i], &data[j]);
            i++;
        }
    }

    swap_int(&data[i], &data[high]);
    return i;
}

static void quick_sort_recursive_range(
    int *data,
    int low,
    int high,
    PivotMethod method,
    Metrics *metrics
)
{
    int pivot_position;

    if (low >= high)
        return;

    pivot_position = partition(data, low, high, method, metrics);
    quick_sort_recursive_range(data, low, pivot_position - 1, method, metrics);
    quick_sort_recursive_range(data, pivot_position + 1, high, method, metrics);
}

static void quick_sort_recursive(int *data, int n, PivotMethod method, Metrics *metrics)
{
    if (n > 1)
        quick_sort_recursive_range(data, 0, n - 1, method, metrics);
}

static void push_range(Range *stack, int *top, int low, int high)
{
    if (low >= high)
        return;
    if (*top + 1 >= STACK_CAPACITY)
    {
        fprintf(stderr, "Stack overflow in iterative quick sort.\n");
        exit(EXIT_FAILURE);
    }
    (*top)++;
    stack[*top].low = low;
    stack[*top].high = high;
}

static void quick_sort_iterative(int *data, int n, PivotMethod method, Metrics *metrics)
{
    Range stack[STACK_CAPACITY];
    int top = -1;

    push_range(stack, &top, 0, n - 1);

    while (top >= 0)
    {
        int low = stack[top].low;
        int high = stack[top].high;
        int pivot_position;

        top--;
        pivot_position = partition(data, low, high, method, metrics);

        if (pivot_position - low > high - pivot_position)
        {
            push_range(stack, &top, low, pivot_position - 1);
            push_range(stack, &top, pivot_position + 1, high);
        }
        else
        {
            push_range(stack, &top, pivot_position + 1, high);
            push_range(stack, &top, low, pivot_position - 1);
        }
    }
}

static const char *method_label(PivotMethod method)
{
    if (method == PIVOT_FIRST)
        return "Quick Sort (First)";
    if (method == PIVOT_RANDOM)
        return "Quick Sort (Random)";
    return "Quick Sort (Median-3)";
}

static const char *method_short_name(PivotMethod method)
{
    if (method == PIVOT_FIRST)
        return "First";
    if (method == PIVOT_RANDOM)
        return "Random";
    return "Median-3";
}

static Metrics sort_copy_and_check(
    const int *original,
    int *work,
    const int *reference,
    int n,
    PivotMethod method
)
{
    Metrics metrics = {0, 0, 0};

    memcpy(work, original, (size_t)n * sizeof(int));
    quick_sort_recursive(work, n, method, &metrics);

    if (!is_sorted(work, n) ||
        (reference != NULL && memcmp(work, reference, (size_t)n * sizeof(int)) != 0))
    {
        fprintf(stderr, "Sorting failed: %s, n = %d\n", method_short_name(method), n);
        exit(EXIT_FAILURE);
    }
    return metrics;
}

static void run_random_experiment(void)
{
    const int sizes[SIZE_COUNT] = {1000, 2000, 4000, 8000, 16000, 20000};
    const PivotMethod methods[METHOD_COUNT] = {PIVOT_FIRST, PIVOT_RANDOM, PIVOT_MEDIAN3};
    Metrics single[SIZE_COUNT][METHOD_COUNT];
    Metrics sum[SIZE_COUNT][METHOD_COUNT];
    int i;
    int k;
    int trial;

    memset(sum, 0, sizeof(sum));

    for (i = 0; i < SIZE_COUNT; i++)
    {
        int n = sizes[i];
        int *original = (int *)checked_malloc((size_t)n * sizeof(int));
        int *work = (int *)checked_malloc((size_t)n * sizeof(int));
        int *reference = (int *)checked_malloc((size_t)n * sizeof(int));

        for (trial = 0; trial < AVERAGE_TRIALS; trial++)
        {
            generate_data(original, n);

            for (k = 0; k < METHOD_COUNT; k++)
            {
                Metrics metrics = sort_copy_and_check(
                    original, work, k == 0 ? NULL : reference, n, methods[k]);

                if (k == 0)
                    memcpy(reference, work, (size_t)n * sizeof(int));
                if (trial == 0)
                    single[i][k] = metrics;
                add_metrics(&sum[i][k], metrics);
            }
        }

        free(original);
        free(work);
        free(reference);
    }

    printf("[1] Random Data - Single Run (Trial 1)\n");
    printf("========================================\n\n");
    for (i = 0; i < SIZE_COUNT; i++)
    {
        printf("Data Size : %d\n\n", sizes[i]);
        for (k = 0; k < METHOD_COUNT; k++)
        {
            printf("%-22s: %llu comparisons\n",
                   method_label(methods[k]), total_comparisons(single[i][k]));
        }
        printf("\n");
    }
    printf("Sorted Check : PASS (all sizes, all pivots, %d trials)\n\n", AVERAGE_TRIALS);

    printf("[2] Random Data - Average of %d Trials\n", AVERAGE_TRIALS);
    printf("========================================\n\n");
    printf("%7s | %12s | %12s | %12s\n", "Size", "First", "Random", "Median-3");
    printf("--------+--------------+--------------+-------------\n");
    for (i = 0; i < SIZE_COUNT; i++)
    {
        printf("%7d", sizes[i]);
        for (k = 0; k < METHOD_COUNT; k++)
        {
            printf(" | %12.1f",
                   (double)total_comparisons(sum[i][k]) / AVERAGE_TRIALS);
        }
        printf("\n");
    }

    printf("\n[3] Comparison Breakdown (Average of %d Trials)\n", AVERAGE_TRIALS);
    printf("========================================\n\n");
    printf("%7s | %-8s | %12s | %12s | %12s | %10s\n",
           "Size", "Pivot", "Total", "Partition", "Pivot-Select", "Partitions");
    printf("--------+----------+--------------+--------------+--------------+-----------\n");
    for (i = 0; i < SIZE_COUNT; i++)
    {
        for (k = 0; k < METHOD_COUNT; k++)
        {
            printf("%7d | %-8s | %12.1f | %12.1f | %12.1f | %10.1f\n",
                   sizes[i],
                   method_short_name(methods[k]),
                   (double)total_comparisons(sum[i][k]) / AVERAGE_TRIALS,
                   (double)sum[i][k].partition_comparisons / AVERAGE_TRIALS,
                   (double)sum[i][k].pivot_comparisons / AVERAGE_TRIALS,
                   (double)sum[i][k].partitions / AVERAGE_TRIALS);
        }
    }
    printf("\n");
}

static void run_sorted_experiment(void)
{
    const int sizes[SORTED_SIZE_COUNT] = {1000, 2000, 4000};
    const PivotMethod methods[METHOD_COUNT] = {PIVOT_FIRST, PIVOT_RANDOM, PIVOT_MEDIAN3};
    int i;
    int k;

    printf("[4] Additional: Already Sorted Input\n");
    printf("========================================\n\n");
    printf("%7s | %12s | %12s | %12s\n", "Size", "First", "Random", "Median-3");
    printf("--------+--------------+--------------+-------------\n");

    for (i = 0; i < SORTED_SIZE_COUNT; i++)
    {
        int n = sizes[i];
        int *original = (int *)checked_malloc((size_t)n * sizeof(int));
        int *work = (int *)checked_malloc((size_t)n * sizeof(int));

        generate_data(original, n);
        qsort(original, (size_t)n, sizeof(int), compare_int);

        printf("%7d", n);
        for (k = 0; k < METHOD_COUNT; k++)
        {
            Metrics metrics = sort_copy_and_check(original, work, NULL, n, methods[k]);
            printf(" | %12llu", total_comparisons(metrics));
        }
        printf("\n");

        free(original);
        free(work);
    }
    printf("\n");
}

static double elapsed_ms(clock_t start, clock_t end)
{
    return (double)(end - start) * 1000.0 / (double)CLOCKS_PER_SEC;
}

static double measure_sort_ms(
    SortFunction sort,
    const int *original,
    int *batch,
    int n,
    Metrics *metrics
)
{
    clock_t start;
    clock_t end;
    int r;

    for (r = 0; r < BONUS_REPEATS; r++)
        memcpy(batch + (size_t)r * (size_t)n, original, (size_t)n * sizeof(int));

    start = clock();
    for (r = 0; r < BONUS_REPEATS; r++)
    {
        Metrics run_metrics = {0, 0, 0};
        sort(batch + (size_t)r * (size_t)n, n, PIVOT_MEDIAN3, &run_metrics);
        if (r == 0)
            *metrics = run_metrics;
    }
    end = clock();

    for (r = 0; r < BONUS_REPEATS; r++)
    {
        if (!is_sorted(batch + (size_t)r * (size_t)n, n))
        {
            fprintf(stderr, "Bonus sorting failed.\n");
            exit(EXIT_FAILURE);
        }
    }

    return elapsed_ms(start, end) / BONUS_REPEATS;
}

static void run_bonus_experiment(void)
{
    int n = BONUS_SIZE;
    int *original = (int *)checked_malloc((size_t)n * sizeof(int));
    int *batch = (int *)checked_malloc((size_t)BONUS_REPEATS * (size_t)n * sizeof(int));
    double recursive_ms = 0.0;
    double iterative_ms = 0.0;
    Metrics recursive_metrics = {0, 0, 0};
    Metrics iterative_metrics = {0, 0, 0};
    int round;

    generate_data(original, n);

    (void)measure_sort_ms(quick_sort_recursive, original, batch, n, &recursive_metrics);
    (void)measure_sort_ms(quick_sort_iterative, original, batch, n, &iterative_metrics);

    for (round = 0; round < BONUS_ROUNDS; round++)
    {
        if (round % 2 == 0)
        {
            recursive_ms += measure_sort_ms(quick_sort_recursive, original, batch, n, &recursive_metrics);
            iterative_ms += measure_sort_ms(quick_sort_iterative, original, batch, n, &iterative_metrics);
        }
        else
        {
            iterative_ms += measure_sort_ms(quick_sort_iterative, original, batch, n, &iterative_metrics);
            recursive_ms += measure_sort_ms(quick_sort_recursive, original, batch, n, &recursive_metrics);
        }
    }

    printf("----------------------------------------\n");
    printf("[Bonus] Recursive vs Iterative Quick Sort\n");
    printf("----------------------------------------\n\n");
    printf("Data Size : %d\n", n);
    printf("Pivot     : Median-of-Three\n");
    printf("Repeats   : %d sorts x %d rounds (order alternated)\n\n",
           BONUS_REPEATS, BONUS_ROUNDS);
    printf("Recursive Quick Sort : %.4f ms\n", recursive_ms / BONUS_ROUNDS);
    printf("Iterative Quick Sort : %.4f ms\n", iterative_ms / BONUS_ROUNDS);
    printf("Comparisons          : %llu / %llu (%s)\n",
           total_comparisons(recursive_metrics),
           total_comparisons(iterative_metrics),
           total_comparisons(recursive_metrics) == total_comparisons(iterative_metrics)
               ? "identical" : "different");
    printf("Sorted Check         : PASS\n");

    free(original);
    free(batch);
}

int main(void)
{
    printf("Quick Sort Pivot Comparison (seed = %llu)\n", (unsigned long long)SEED);
    printf("Comparisons = partition comparisons + Median-of-Three selection comparisons\n\n");

    run_random_experiment();
    run_sorted_experiment();
    run_bonus_experiment();
    return 0;
}