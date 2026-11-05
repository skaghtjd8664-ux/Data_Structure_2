#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KEY_MIN 0
#define KEY_MAX 1000000
#define SIZE_COUNT 6
#define BONUS_SIZE 10000
#define BONUS_REPEATS 20

typedef enum {
    PIVOT_FIRST,
    PIVOT_RANDOM,
    PIVOT_MEDIAN3
} PivotMethod;

typedef struct {
    unsigned long long comparisons;
} Metrics;

typedef struct {
    int low;
    int high;
} Range;

static int random_value(void)
{
    return rand() % (KEY_MAX - KEY_MIN + 1) + KEY_MIN;
}

static void generate_data(int *data, int n)
{
    int i;
    for (i = 0; i < n; i++)
        data[i] = random_value();
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

static int choose_pivot_index(
    const int *data,
    int low,
    int high,
    PivotMethod method,
    Metrics *metrics
)
{
    int middle;
    int first;
    int last;

    if (method == PIVOT_FIRST)
        return low;

    if (method == PIVOT_RANDOM)
        return low + rand() % (high - low + 1);

    middle = low + (high - low) / 2;
    first = low;
    last = high;

    /* 중앙값 결정에 사용한 데이터 비교도 측정한다. */
    metrics->comparisons++;
    if (data[first] < data[middle])
    {
        metrics->comparisons++;
        if (data[middle] < data[last])
            return middle;

        metrics->comparisons++;
        if (data[first] < data[last])
            return last;
        return first;
    }

    metrics->comparisons++;
    if (data[first] < data[last])
        return first;

    metrics->comparisons++;
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

    pivot_index = choose_pivot_index(data, low, high, method, metrics);
    pivot_value = data[pivot_index];
    swap_int(&data[pivot_index], &data[high]);

    i = low;
    for (j = low; j < high; j++)
    {
        metrics->comparisons++;
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

static Metrics quick_sort_recursive(
    int *data,
    int n,
    PivotMethod method
)
{
    Metrics metrics = {0};

    if (n > 1)
        quick_sort_recursive_range(data, 0, n - 1, method, &metrics);

    return metrics;
}

static Metrics quick_sort_iterative(
    int *data,
    int n,
    PivotMethod method
)
{
    Metrics metrics = {0};
    Range *stack;
    int top = -1;

    if (n <= 1)
        return metrics;

    stack = (Range *)malloc((size_t)n * sizeof(Range));
    if (stack == NULL)
    {
        fprintf(stderr, "Stack allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    stack[++top].low = 0;
    stack[top].high = n - 1;

    while (top >= 0)
    {
        int low = stack[top].low;
        int high = stack[top].high;
        int pivot_position;
        top--;

        if (low >= high)
            continue;

        pivot_position = partition(data, low, high, method, &metrics);

        if (low < pivot_position - 1)
        {
            stack[++top].low = low;
            stack[top].high = pivot_position - 1;
        }

        if (pivot_position + 1 < high)
        {
            stack[++top].low = pivot_position + 1;
            stack[top].high = high;
        }
    }

    free(stack);
    return metrics;
}

static double elapsed_ms(clock_t start, clock_t end)
{
    return (double)(end - start) * 1000.0 / (double)CLOCKS_PER_SEC;
}

static const char *pivot_name(PivotMethod method)
{
    if (method == PIVOT_FIRST)
        return "First";
    if (method == PIVOT_RANDOM)
        return "Random";
    return "Median-3";
}

int main(void)
{
    const int sizes[SIZE_COUNT] = {1000, 2000, 4000, 8000, 16000, 20000};
    int i;

    srand((unsigned int)time(NULL));

    printf("Quick Sort Pivot Comparison\n");
    printf("============================\n");
    printf("Comparison count includes partition comparisons and Median-of-Three comparisons.\n\n");

    for (i = 0; i < SIZE_COUNT; i++)
    {
        int n = sizes[i];
        int *original = (int *)malloc((size_t)n * sizeof(int));
        int *work = (int *)malloc((size_t)n * sizeof(int));
        PivotMethod method;

        if (original == NULL || work == NULL)
        {
            fprintf(stderr, "Memory allocation failed.\n");
            free(original);
            free(work);
            return EXIT_FAILURE;
        }

        generate_data(original, n);

        printf("Data Size : %d\n\n", n);

        for (method = PIVOT_FIRST; method <= PIVOT_MEDIAN3; method++)
        {
            Metrics metrics;

            memcpy(work, original, (size_t)n * sizeof(int));
            metrics = quick_sort_recursive(work, n, method);

            printf("Quick Sort (%-7s) : %llu comparisons\n",
                   pivot_name(method), metrics.comparisons);

            if (!is_sorted(work, n))
            {
                fprintf(stderr, "Sorting failed for %s.\n", pivot_name(method));
                free(original);
                free(work);
                return EXIT_FAILURE;
            }
        }

        printf("Sorted Check         : PASS\n\n");
        free(original);
        free(work);
    }

    {
        int *original = (int *)malloc((size_t)BONUS_SIZE * sizeof(int));
        int *recursive_data = (int *)malloc((size_t)BONUS_SIZE * sizeof(int));
        int *iterative_data = (int *)malloc((size_t)BONUS_SIZE * sizeof(int));
        double recursive_total = 0.0;
        double iterative_total = 0.0;
        int repeat;

        if (original == NULL || recursive_data == NULL || iterative_data == NULL)
        {
            fprintf(stderr, "Memory allocation failed.\n");
            free(original);
            free(recursive_data);
            free(iterative_data);
            return EXIT_FAILURE;
        }

        generate_data(original, BONUS_SIZE);

        for (repeat = 0; repeat < BONUS_REPEATS; repeat++)
        {
            clock_t start;
            clock_t end;

            memcpy(recursive_data, original, (size_t)BONUS_SIZE * sizeof(int));
            start = clock();
            (void)quick_sort_recursive(recursive_data, BONUS_SIZE, PIVOT_MEDIAN3);
            end = clock();
            recursive_total += elapsed_ms(start, end);

            memcpy(iterative_data, original, (size_t)BONUS_SIZE * sizeof(int));
            start = clock();
            (void)quick_sort_iterative(iterative_data, BONUS_SIZE, PIVOT_MEDIAN3);
            end = clock();
            iterative_total += elapsed_ms(start, end);
        }

        printf("----------------------------------------\n");
        printf("Bonus: Recursive vs Iterative Quick Sort\n");
        printf("Data Size : %d\n", BONUS_SIZE);
        printf("Pivot     : Median-of-Three\n");
        printf("Repeats   : %d\n\n", BONUS_REPEATS);
        printf("Recursive Quick Sort : %.6f ms\n", recursive_total / BONUS_REPEATS);
        printf("Iterative Quick Sort : %.6f ms\n", iterative_total / BONUS_REPEATS);
        printf("Sorted Check         : %s\n",
               is_sorted(recursive_data, BONUS_SIZE) &&
               is_sorted(iterative_data, BONUS_SIZE) ? "PASS" : "FAIL");

        free(original);
        free(recursive_data);
        free(iterative_data);
    }

    return 0;
}
