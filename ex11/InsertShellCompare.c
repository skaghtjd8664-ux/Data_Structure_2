#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIN_DATA_SIZE 1000
#define MAX_POWER_SIZE 16000
#define EXTRA_DATA_SIZE 20000
#define KEY_MIN 0
#define KEY_MAX 1000000

typedef struct
{
    unsigned long long insertion;
    unsigned long long shell_original;
    unsigned long long shell_knuth;
} ComparisonResult;

static void memory_error(void)
{
    fprintf(stderr, "Memory allocation failed.\n");
    exit(EXIT_FAILURE);
}

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

static void copy_data(int *destination, const int *source, int n)
{
    memcpy(destination, source, (size_t)n * sizeof(int));
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

static unsigned long long insertion_sort(int *data, int n)
{
    unsigned long long comparisons = 0;
    int i;

    for (i = 1; i < n; i++)
    {
        int current = data[i];
        int j = i - 1;

        while (j >= 0)
        {
            comparisons++;

            if (data[j] > current)
            {
                data[j + 1] = data[j];
                j--;
            }
            else
            {
                break;
            }
        }

        data[j + 1] = current;
    }

    return comparisons;
}

static unsigned long long shell_sort_with_gap_sequence(
    int *data,
    int n,
    int use_knuth
)
{
    unsigned long long comparisons = 0;
    int gap;

    if (use_knuth)
    {
        gap = 1;

        while (gap < n / 3)
            gap = 3 * gap + 1;

        while (gap >= 1)
        {
            int i;

            for (i = gap; i < n; i++)
            {
                int current = data[i];
                int j = i;

                while (j >= gap)
                {
                    comparisons++;

                    if (data[j - gap] > current)
                    {
                        data[j] = data[j - gap];
                        j -= gap;
                    }
                    else
                    {
                        break;
                    }
                }

                data[j] = current;
            }

            gap = (gap - 1) / 3;
        }
    }
    else
    {
        gap = n / 2;

        while (gap >= 1)
        {
            int i;

            for (i = gap; i < n; i++)
            {
                int current = data[i];
                int j = i;

                while (j >= gap)
                {
                    comparisons++;

                    if (data[j - gap] > current)
                    {
                        data[j] = data[j - gap];
                        j -= gap;
                    }
                    else
                    {
                        break;
                    }
                }

                data[j] = current;
            }

            gap /= 2;
        }
    }

    return comparisons;
}

static ComparisonResult run_experiment(
    const int *original,
    int n
)
{
    int *work;
    ComparisonResult result;

    work = (int *)malloc((size_t)n * sizeof(int));
    if (work == NULL)
        memory_error();

    result.insertion = 0;
    result.shell_original = 0;
    result.shell_knuth = 0;

    copy_data(work, original, n);
    result.insertion = insertion_sort(work, n);
    if (!is_sorted(work, n))
    {
        fprintf(stderr, "Insertion sort failed.\n");
        free(work);
        exit(EXIT_FAILURE);
    }

    copy_data(work, original, n);
    result.shell_original =
        shell_sort_with_gap_sequence(work, n, 0);
    if (!is_sorted(work, n))
    {
        fprintf(stderr, "Shell sort with original gaps failed.\n");
        free(work);
        exit(EXIT_FAILURE);
    }

    copy_data(work, original, n);
    result.shell_knuth =
        shell_sort_with_gap_sequence(work, n, 1);
    if (!is_sorted(work, n))
    {
        fprintf(stderr, "Shell sort with Knuth gaps failed.\n");
        free(work);
        exit(EXIT_FAILURE);
    }

    free(work);
    return result;
}

int main(void)
{
    const int sizes[] = {
        1000,
        2000,
        4000,
        8000,
        16000,
        20000
    };
    const int size_count = (int)(sizeof(sizes) / sizeof(sizes[0]));
    int i;

    srand((unsigned int)time(NULL));

    printf("Insertion Sort and Shell Sort Comparison\n");
    printf("=========================================\n");
    printf("Comparison count excludes data generation and array copying.\n");
    printf("All sorting results are checked for ascending order.\n\n");

    for (i = 0; i < size_count; i++)
    {
        int n = sizes[i];
        int *original;
        ComparisonResult result;

        original = (int *)malloc((size_t)n * sizeof(int));
        if (original == NULL)
            memory_error();

        generate_data(original, n);
        result = run_experiment(original, n);

        printf("Data Size : %d\n\n", n);
        printf("Insertion Sort        : %llu comparisons\n",
               result.insertion);
        printf("Shell Sort (Original) : %llu comparisons\n",
               result.shell_original);
        printf("Shell Sort (Knuth)    : %llu comparisons\n",
               result.shell_knuth);
        printf("Sorted Check          : PASS\n\n");

        free(original);
    }

    return 0;
}
