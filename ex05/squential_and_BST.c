#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_MIN        0
#define DATA_MAX        1000
#define NUM_DATA        100
#define NUM_SEARCH      50

/* ------------------------------ BST 노드 ------------------------------ */

typedef struct BSTNode
{
    int data;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

/* ------------------------------ 1. 데이터 생성 ------------------------------ */

/* 배열안에 중복된 숫자가 있는지 확인 */
static int check_duplicate(const int *arr, int count, int value)
{
    int i;
    for (i = 0; i < count; i++)
    {
        if (arr[i] == value)
            return 1;
    }
    return 0;
}

/* 서로 다른 정수를 배열을 생성해서 배열에 저장 */
static void generate_numbers(int *arr, int n)
{
    int count = 0;
    int value;

    while (count < n)
    {
        value = rand() % (DATA_MAX - DATA_MIN + 1) + DATA_MIN;
        if (!check_duplicate(arr, count, value))
        {
            arr[count] = value;
            count++;
        }
    }
}

/* 탐색할 정수 50개 생성 */
static void generate_search_keys(int *arr, int n)
{
    int i;
    for (i = 0; i < n; i++)
        arr[i] = rand() % (DATA_MAX - DATA_MIN + 1) + DATA_MIN;
}

static BSTNode *bst_create_node(int value)
{
    BSTNode *node = (BSTNode *)malloc(sizeof(BSTNode));
    node->data = value;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static BSTNode *bst_insert(BSTNode *root, int value, long *comparisons)
{
    BSTNode *cur;
    BSTNode *parent;

    if (root == NULL)
        return bst_create_node(value);
    parent = NULL;
    cur = root;
    while (cur != NULL)
    {
        (*comparisons)++;
        parent = cur;
        if (value < cur->data)
            cur = cur->left;
        else
            cur = cur->right;
    }

    if (value < parent->data)
        parent->left = bst_create_node(value);
    else
        parent->right = bst_create_node(value);
    return root;
}

static int bst_search(BSTNode *root, int value, long *comparisons)
{
    BSTNode *cur = root;

    while (cur != NULL)
    {
        (*comparisons)++;
        if (value == cur->data)
            return 1;
        else if (value < cur->data)
            cur = cur->left;
        else
            cur = cur->right;
    }
    return 0;
}

static int bst_height(BSTNode *root)
{
    int lh, rh;

    if (root == NULL)
        return 0;

    lh = bst_height(root->left);
    rh = bst_height(root->right);

    if (lh > rh)
        return 1 + lh;
    else
        return 1 + rh;
}

static void bst_free(BSTNode *root)
{
    if (root == NULL)
        return;
    bst_free(root->left);
    bst_free(root->right); 
    free(root);
}

static int sequential_search(const int *arr, int n, int value, long *comparisons)
{
    int i;
    for (i = 0; i < n; i++)
    {
        (*comparisons)++;
        if (arr[i] == value)
            return 1;
    }
    return 0;
}


int main(void)
{
    int array[NUM_DATA];
    int search_keys[NUM_SEARCH];
    BSTNode *root = NULL;
    long bst_build_comparisons = 0;
    long seq_total = 0, bst_total = 0;
    int seq_counts[NUM_SEARCH];
    long bst_counts[NUM_SEARCH];
    int found_flags_seq[NUM_SEARCH];
    int found_flags_bst[NUM_SEARCH];
    int i;

    srand((unsigned int)time(NULL));

    /* 서로 다른 100개의 정수를 생성해서 배열에 저장*/
    generate_numbers(array, NUM_DATA);
    printf("=== 생성된 서로 다른 100개의 정수 ===\n");
    for (i = 0; i < NUM_DATA; i++)
    {
        printf("%4d", array[i]);
        if ((i + 1) % 10 == 0)
            printf("\n");
        else
            printf(" ");
    }
    printf("\n");

    /* 동일한 100개의 정수를 발생 순서대로 BST에 삽입하면서 비교 횟수 측정 */
    for (i = 0; i < NUM_DATA; i++)
        root = bst_insert(root, array[i], &bst_build_comparisons);

    printf("=== BST 생성 결과 ===\n");
    printf("BST 생성 중 발생한 총 비교 횟수 : %ld\n", bst_build_comparisons);
    printf("BST 높이(height)                : %d\n", bst_height(root));
    printf("\n");

    /* 50개의 탐색 대상 숫자 생성*/
    generate_search_keys(search_keys, NUM_SEARCH);

    printf("=== 생성된 50개의 탐색 대상 ===\n");
    for (i = 0; i < NUM_SEARCH; i++)
    {
        printf("%4d", search_keys[i]);
        if ((i + 1) % 10 == 0)
            printf("\n");
        else
            printf(" ");
    }
    printf("\n");

    /* 탐색 성공여부와 순차 탐색과 BST 탐색 비교 횟수*/
    printf("=== 각 탐색 대상에 대한 순차 탐색과 BST 탐색 결과 ===\n");
    printf("%-4s %-8s %-8s %-8s %-11s %-11s\n", "No.", "Key", "SeqCmp", "BSTCmp", "Result(Seq)", "Result(BST)");
    printf("-------------------------------------------------------------------\n");

    for (i = 0; i < NUM_SEARCH; i++)
    {
        long sc = 0, bc = 0;
        int key = search_keys[i];

        found_flags_seq[i] = sequential_search(array, NUM_DATA, key, &sc);
        found_flags_bst[i] = bst_search(root, key, &bc);

        seq_counts[i] = (int)sc;
        bst_counts[i] = bc;

        seq_total += sc;
        bst_total += bc;

        char seq_result[16];
        char bst_result[16];
 
        if (found_flags_seq[i])
            strcpy(seq_result, "Found");
        else
            strcpy(seq_result, "Not Found");
 
        if (found_flags_bst[i])
            strcpy(bst_result, "Found");
        else
            strcpy(bst_result, "Not Found");
 
        printf("%-4d %-8d %-8d %-8ld %-10s %-10s\n", i + 1, key, seq_counts[i], bst_counts[i], seq_result, bst_result);
    }
    printf("\n");

    printf("=== 순차 탐색과 BST 탐색의 총 비교 횟수와 평균 횟수 ===\n");

    printf("Sequential Search\n");
    printf("  Total comparisons   : %ld\n", seq_total);
    printf("  Average comparisons : %.2f\n", (double)seq_total / NUM_SEARCH);

    printf("\nBST Search\n");
    printf("  Total comparisons   : %ld\n", bst_total);
    printf("  Average comparisons : %.2f\n", (double)bst_total / NUM_SEARCH);
    printf("\n");

    printf("=== 이진 탐색 트리 생성 비용 ===\n");
    printf("BST 생성 비용 (총 비교 횟수)         : %ld\n", bst_build_comparisons);
    printf("BST 탐색 50회 총 비교 횟수           : %ld\n", bst_total);
    printf("BST 생성+탐색 합계                   : %ld\n", bst_build_comparisons + bst_total);
    printf("순차 탐색 50회 총 비교 횟수           : %ld\n", seq_total);

    bst_free(root);
    return 0;
}