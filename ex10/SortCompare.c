#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_COUNT 10000
#define KEY_MIN 0
#define KEY_MAX 1000000
#ifndef RANDOM_TRIALS
#define RANDOM_TRIALS 100
#endif
#define PARTIAL_SWAPS 500

typedef struct {
    int key1;
    int key2;
    int original_index;
} Data;

typedef struct {
    unsigned long long comparisons;
    unsigned long long movements;
} Metrics;

static void die(void)
{
    fprintf(stderr, "Memory allocation failed.\n");
    exit(EXIT_FAILURE);
}

static void swap_data(Data *a, Data *b, Metrics *m)
{
    Data temp = *a;
    *a = *b;
    *b = temp;
    m->movements += 3;
}

static int is_sorted(const Data a[], int n)
{
    int i;
    for (i = 1; i < n; i++)
        if (a[i - 1].key1 > a[i].key1)
            return 0;
    return 1;
}

static int is_stable(const Data a[], int n)
{
    int i;
    for (i = 1; i < n; i++) {
        if (a[i - 1].key1 == a[i].key1 &&
            a[i - 1].original_index > a[i].original_index)
            return 0;
    }
    return 1;
}

static void selection_sort(Data a[], int n, Metrics *m)
{
    int i, j, min_index;
    for (i = 0; i < n - 1; i++) {
        min_index = i;
        for (j = i + 1; j < n; j++) {
            m->comparisons++;
            if (a[j].key1 < a[min_index].key1)
                min_index = j;
        }
        if (min_index != i)
            swap_data(&a[i], &a[min_index], m);
    }
}

static void bubble_sort(Data a[], int n, Metrics *m, int early_exit)
{
    int end, i;
    int swapped;
    for (end = n - 1; end > 0; end--) {
        swapped = 0;
        for (i = 0; i < end; i++) {
            m->comparisons++;
            if (a[i].key1 > a[i + 1].key1) {
                swap_data(&a[i], &a[i + 1], m);
                swapped = 1;
            }
        }
        if (early_exit && !swapped)
            break;
    }
}

static void insertion_sort(Data a[], int n, Metrics *m, int use_greater_equal)
{
    int i, j;
    Data current;

    for (i = 1; i < n; i++) {
        current = a[i];
        m->movements++;
        j = i - 1;

        while (j >= 0) {
            m->comparisons++;
            if (use_greater_equal) {
                if (a[j].key1 < current.key1)
                    break;
            } else {
                if (a[j].key1 <= current.key1)
                    break;
            }
            a[j + 1] = a[j];
            m->movements++;
            j--;
        }
        a[j + 1] = current;
        m->movements++;
    }
}

static int compare_key1_index(const void *x, const void *y)
{
    const Data *a = (const Data *)x;
    const Data *b = (const Data *)y;
    if (a->key1 != b->key1)
        return a->key1 < b->key1 ? -1 : 1;
    if (a->original_index != b->original_index)
        return a->original_index < b->original_index ? -1 : 1;
    return 0;
}

static void generate_random_data(Data a[], int n)
{
    int i;
    for (i = 0; i < n; i++) {
        a[i].key1 = rand() % (KEY_MAX + 1);
        a[i].key2 = rand() % (KEY_MAX + 1);
        a[i].original_index = i;
    }
}

static void make_partially_sorted(const Data sorted[], Data out[], int n)
{
    int i;
    memcpy(out, sorted, sizeof(Data) * (size_t)n);
    for (i = 0; i < PARTIAL_SWAPS; i++) {
        int x = rand() % n;
        int y = rand() % n;
        Data temp = out[x];
        out[x] = out[y];
        out[y] = temp;
    }
}

static void print_metric(const char *name, Metrics m)
{
    printf("%s\n", name);
    printf("Comparisons : %llu\n", m.comparisons);
    printf("Movements   : %llu\n", m.movements);
}

static void run_one_state(const char *title, const Data source[], int n)
{
    Data *a = malloc(sizeof(Data) * (size_t)n);
    Metrics s = {0, 0}, b = {0, 0}, be = {0, 0}, ins = {0, 0};
    if (a == NULL) die();

    memcpy(a, source, sizeof(Data) * (size_t)n);
    selection_sort(a, n, &s);
    printf("\n%s\n", title);
    print_metric("Selection Sort", s);
    printf("Sorted       : %s\n", is_sorted(a, n) ? "YES" : "NO");

    memcpy(a, source, sizeof(Data) * (size_t)n);
    bubble_sort(a, n, &b, 0);
    print_metric("Bubble Sort", b);
    printf("Sorted       : %s\n", is_sorted(a, n) ? "YES" : "NO");

    memcpy(a, source, sizeof(Data) * (size_t)n);
    bubble_sort(a, n, &be, 1);
    print_metric("Bubble Sort (Early Termination)", be);
    printf("Sorted       : %s\n", is_sorted(a, n) ? "YES" : "NO");

    memcpy(a, source, sizeof(Data) * (size_t)n);
    insertion_sort(a, n, &ins, 0);
    print_metric("Insertion Sort", ins);
    printf("Sorted       : %s\n", is_sorted(a, n) ? "YES" : "NO");

    free(a);
}

static void add_metrics(Metrics *total, Metrics current)
{
    total->comparisons += current.comparisons;
    total->movements += current.movements;
}

static void print_average(const char *name, Metrics total, int trials)
{
    printf("\n%s\n", name);
    printf("Total Comparisons   : %llu\n", total.comparisons);
    printf("Average Comparisons : %.2f\n",
           (double)total.comparisons / trials);
    printf("Average Movements   : %.2f\n",
           (double)total.movements / trials);
}

static void stability_test(void)
{
    Data input[] = {
        {527, 100, 0}, {125, 300, 1}, {527, 200, 2},
        {892, 400, 3}, {125, 500, 4}, {527, 600, 5},
        {125, 700, 6}, {892, 800, 7}
    };
    Data a[sizeof(input) / sizeof(input[0])];
    Metrics m = {0, 0};
    int n = (int)(sizeof(input) / sizeof(input[0]));

    memcpy(a, input, sizeof(input));
    selection_sort(a, n, &m);
    printf("Selection Sort       : %s\n", is_stable(a, n) ? "Stable" : "Not Stable");

    memcpy(a, input, sizeof(input));
    bubble_sort(a, n, &m, 1);
    printf("Bubble Sort          : %s\n", is_stable(a, n) ? "Stable" : "Not Stable");

    memcpy(a, input, sizeof(input));
    insertion_sort(a, n, &m, 0);
    printf("Insertion Sort (>)   : %s\n", is_stable(a, n) ? "Stable" : "Not Stable");

    memcpy(a, input, sizeof(input));
    insertion_sort(a, n, &m, 1);
    printf("Insertion Sort (>=)  : %s\n", is_stable(a, n) ? "Stable" : "Not Stable");
}

int main(void)
{
    Data *base = malloc(sizeof(Data) * DATA_COUNT);
    Data *copy = malloc(sizeof(Data) * DATA_COUNT);
    Data *sorted = malloc(sizeof(Data) * DATA_COUNT);
    Data *reverse = malloc(sizeof(Data) * DATA_COUNT);
    Data *partial = malloc(sizeof(Data) * DATA_COUNT);
    Metrics selection_total = {0, 0};
    Metrics bubble_total = {0, 0};
    Metrics early_total = {0, 0};
    Metrics insertion_total = {0, 0};
    int trial, i;

    if (base == NULL || copy == NULL || sorted == NULL ||
        reverse == NULL || partial == NULL)
        die();

    srand((unsigned int)time(NULL));

    for (trial = 0; trial < RANDOM_TRIALS; trial++) {
        Metrics m;
        generate_random_data(base, DATA_COUNT);

        memcpy(copy, base, sizeof(Data) * DATA_COUNT);
        m = (Metrics){0, 0}; selection_sort(copy, DATA_COUNT, &m); add_metrics(&selection_total, m);
        if (!is_sorted(copy, DATA_COUNT)) return EXIT_FAILURE;

        memcpy(copy, base, sizeof(Data) * DATA_COUNT);
        m = (Metrics){0, 0}; bubble_sort(copy, DATA_COUNT, &m, 0); add_metrics(&bubble_total, m);
        if (!is_sorted(copy, DATA_COUNT)) return EXIT_FAILURE;

        memcpy(copy, base, sizeof(Data) * DATA_COUNT);
        m = (Metrics){0, 0}; bubble_sort(copy, DATA_COUNT, &m, 1); add_metrics(&early_total, m);
        if (!is_sorted(copy, DATA_COUNT)) return EXIT_FAILURE;

        memcpy(copy, base, sizeof(Data) * DATA_COUNT);
        m = (Metrics){0, 0}; insertion_sort(copy, DATA_COUNT, &m, 0); add_metrics(&insertion_total, m);
        if (!is_sorted(copy, DATA_COUNT)) return EXIT_FAILURE;
    }

    printf("Data Count : %d\n", DATA_COUNT);
    printf("Random Data\nNumber of Trials : %d\n", RANDOM_TRIALS);
    print_average("Selection Sort", selection_total, RANDOM_TRIALS);
    print_average("Bubble Sort", bubble_total, RANDOM_TRIALS);
    print_average("Bubble Sort (Early Termination)", early_total, RANDOM_TRIALS);
    print_average("Insertion Sort", insertion_total, RANDOM_TRIALS);

    generate_random_data(base, DATA_COUNT);
    memcpy(sorted, base, sizeof(Data) * DATA_COUNT);
    qsort(sorted, DATA_COUNT, sizeof(Data), compare_key1_index);
    for (i = 0; i < DATA_COUNT; i++)
        reverse[i] = sorted[DATA_COUNT - 1 - i];
    make_partially_sorted(sorted, partial, DATA_COUNT);

    run_one_state("------------------------------------\nSorted Data", sorted, DATA_COUNT);
    run_one_state("------------------------------------\nReverse-Sorted Data", reverse, DATA_COUNT);
    run_one_state("------------------------------------\nPartially Sorted Data", partial, DATA_COUNT);

    printf("\n------------------------------------\nStability Verification\n");
    stability_test();

    free(base);
    free(copy);
    free(sorted);
    free(reverse);
    free(partial);
    return 0;
}
