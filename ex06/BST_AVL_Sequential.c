#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_MIN        0
#define DATA_MAX        1000
#define NUM_ATTEMPTS    100
#define NUM_SEARCH      50

typedef struct BSTNode
{
    int data;
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

typedef struct AVLNode
{
    int data;
    int height;
    struct AVLNode *left;
    struct AVLNode *right;
} AVLNode;

static int generate_random_value(void)
{
    return rand() % (DATA_MAX - DATA_MIN + 1) + DATA_MIN;
}

static int array_insert(
    int *array,
    int *size,
    int value,
    long *comparisons
)
{
    int i;

    for (i = 0; i < *size; i++)
    {
        (*comparisons)++;

        if (array[i] == value)
            return 0;
    }

    array[*size] = value;
    (*size)++;

    return 1;
}

static int sequential_search(
    const int *array,
    int size,
    int value,
    long *comparisons
)
{
    int i;

    for (i = 0; i < size; i++)
    {
        (*comparisons)++;

        if (array[i] == value)
            return 1;
    }

    return 0;
}

static BSTNode *bst_create_node(int value)
{
    BSTNode *node;

    node = (BSTNode *)malloc(sizeof(BSTNode));

    if (node == NULL)
    {
        fprintf(stderr, "BST node memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    node->data = value;
    node->left = NULL;
    node->right = NULL;

    return node;
}

static BSTNode *bst_insert(
    BSTNode *root,
    int value,
    long *comparisons,
    int *inserted
)
{
    BSTNode *cur;
    BSTNode *parent;

    *inserted = 0;

    if (root == NULL)
    {
        *inserted = 1;
        return bst_create_node(value);
    }

    cur = root;
    parent = NULL;

    while (cur != NULL)
    {
        (*comparisons)++;
        parent = cur;

        if (value == cur->data)
        {
            return root;
        }
        else if (value < cur->data)
        {
            cur = cur->left;
        }
        else
        {
            cur = cur->right;
        }
    }

    if (value < parent->data)
        parent->left = bst_create_node(value);
    else
        parent->right = bst_create_node(value);

    *inserted = 1;

    return root;
}

static int bst_search(
    BSTNode *root,
    int value,
    long *comparisons
)
{
    BSTNode *cur;

    cur = root;

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
    int left_height;
    int right_height;

    if (root == NULL)
        return 0;

    left_height = bst_height(root->left);
    right_height = bst_height(root->right);

    if (left_height > right_height)
        return left_height + 1;
    else
        return right_height + 1;
}

static void bst_free(BSTNode *root)
{
    if (root == NULL)
        return;

    bst_free(root->left);
    bst_free(root->right);

    free(root);
}

static AVLNode *avl_create_node(int value)
{
    AVLNode *node;

    node = (AVLNode *)malloc(sizeof(AVLNode));

    if (node == NULL)
    {
        fprintf(stderr, "AVL node memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    node->data = value;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;

    return node;
}

static int avl_height(AVLNode *root)
{
    if (root == NULL)
        return 0;

    return root->height;
}

static int avl_max(int a, int b)
{
    if (a > b)
        return a;
    else
        return b;
}

static AVLNode *avl_rotate_right(AVLNode *y)
{
    AVLNode *x;
    AVLNode *t2;

    x = y->left;
    t2 = x->right;

    x->right = y;
    y->left = t2;

    y->height =
        1 + avl_max(
            avl_height(y->left),
            avl_height(y->right)
        );

    x->height =
        1 + avl_max(
            avl_height(x->left),
            avl_height(x->right)
        );

    return x;
}

static AVLNode *avl_rotate_left(AVLNode *x)
{
    AVLNode *y;
    AVLNode *t2;

    y = x->right;
    t2 = y->left;

    y->left = x;
    x->right = t2;

    x->height =
        1 + avl_max(
            avl_height(x->left),
            avl_height(x->right)
        );

    y->height =
        1 + avl_max(
            avl_height(y->left),
            avl_height(y->right)
        );

    return y;
}

static AVLNode *avl_insert(
    AVLNode *root,
    int value,
    long *comparisons,
    int *inserted
)
{
    int balance;

    if (root == NULL)
    {
        *inserted = 1;
        return avl_create_node(value);
    }

    (*comparisons)++;

    if (value == root->data)
    {
        *inserted = 0;
        return root;
    }
    else if (value < root->data)
    {
        root->left =
            avl_insert(
                root->left,
                value,
                comparisons,
                inserted
            );
    }
    else
    {
        root->right =
            avl_insert(
                root->right,
                value,
                comparisons,
                inserted
            );
    }

    if (!(*inserted))
        return root;

    root->height =
        1 + avl_max(
            avl_height(root->left),
            avl_height(root->right)
        );

    balance =
        avl_height(root->left) -
        avl_height(root->right);

    if (balance > 1 && value < root->left->data)
        return avl_rotate_right(root);

    if (balance < -1 && value > root->right->data)
        return avl_rotate_left(root);

    if (balance > 1 && value > root->left->data)
    {
        root->left = avl_rotate_left(root->left);
        return avl_rotate_right(root);
    }

    if (balance < -1 && value < root->right->data)
    {
        root->right = avl_rotate_right(root->right);
        return avl_rotate_left(root);
    }

    return root;
}

static int avl_search(
    AVLNode *root,
    int value,
    long *comparisons
)
{
    AVLNode *cur;

    cur = root;

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

static void avl_free(AVLNode *root)
{
    if (root == NULL)
        return;

    avl_free(root->left);
    avl_free(root->right);

    free(root);
}

int main(void)
{
    int generated_values[NUM_ATTEMPTS];
    int array[NUM_ATTEMPTS];
    int search_keys[NUM_SEARCH];

    int array_size = 0;
    int duplicate_count = 0;

    BSTNode *bst_root = NULL;
    AVLNode *avl_root = NULL;

    long array_build_comparisons = 0;
    long bst_build_comparisons = 0;
    long avl_build_comparisons = 0;

    long sequential_total = 0;
    long bst_total = 0;
    long avl_total = 0;

    int sequential_counts[NUM_SEARCH];
    long bst_counts[NUM_SEARCH];
    long avl_counts[NUM_SEARCH];

    int found_sequential[NUM_SEARCH];
    int found_bst[NUM_SEARCH];
    int found_avl[NUM_SEARCH];

    int i;

    srand((unsigned int)time(NULL));

    for (i = 0; i < NUM_ATTEMPTS; i++)
    {
        generated_values[i] = generate_random_value();
    }

    printf("============================================================\n");
    printf("Generated 100 random values\n");
    printf("============================================================\n");

    for (i = 0; i < NUM_ATTEMPTS; i++)
    {
        printf("%4d", generated_values[i]);

        if ((i + 1) % 10 == 0)
            printf("\n");
        else
            printf(" ");
    }

    printf("\n");

    for (i = 0; i < NUM_ATTEMPTS; i++)
    {
        int value;
        int array_inserted;
        int bst_inserted;
        int avl_inserted;

        value = generated_values[i];

        array_inserted =
            array_insert(
                array,
                &array_size,
                value,
                &array_build_comparisons
            );

        bst_root =
            bst_insert(
                bst_root,
                value,
                &bst_build_comparisons,
                &bst_inserted
            );

        avl_root =
            avl_insert(
                avl_root,
                value,
                &avl_build_comparisons,
                &avl_inserted
            );

        if (!array_inserted)
            duplicate_count++;

        if (array_inserted != bst_inserted ||
            array_inserted != avl_inserted)
        {
            fprintf(
                stderr,
                "Error: insertion results are inconsistent.\n"
            );

            bst_free(bst_root);
            avl_free(avl_root);

            return EXIT_FAILURE;
        }
    }

    printf("============================================================\n");
    printf("Construction Result\n");
    printf("============================================================\n");

    printf("Stored values              : %d\n", array_size);
    printf("Duplicate values           : %d\n", duplicate_count);
    printf("Array comparisons          : %ld\n",
           array_build_comparisons);
    printf("BST comparisons            : %ld\n",
           bst_build_comparisons);
    printf("AVL comparisons            : %ld\n",
           avl_build_comparisons);

    printf("\n");

    printf("============================================================\n");
    printf("Structure\n");
    printf("============================================================\n");

    printf("Array length               : %d\n", array_size);
    printf("BST height                 : %d\n",
           bst_height(bst_root));
    printf("AVL height                 : %d\n",
           avl_height(avl_root));

    printf("\n");

    for (i = 0; i < NUM_SEARCH; i++)
    {
        search_keys[i] = generate_random_value();
    }

    printf("============================================================\n");
    printf("Generated 50 search keys\n");
    printf("============================================================\n");

    for (i = 0; i < NUM_SEARCH; i++)
    {
        printf("%4d", search_keys[i]);

        if ((i + 1) % 10 == 0)
            printf("\n");
        else
            printf(" ");
    }

    printf("\n");

    printf("============================================================\n");
    printf("Search Results\n");
    printf("============================================================\n");

    printf(
        "%-4s %-8s %-8s %-8s %-8s %-15s %-15s %-15s\n",
        "No.",
        "Key",
        "SeqCmp",
        "BSTCmp",
        "AVLCmp",
        "Result(Seq)",
        "Result(BST)",
        "Result(AVL)"
    );

    printf(
        "--------------------------------------------------------------------------------\n"
    );

    for (i = 0; i < NUM_SEARCH; i++)
    {
        long sequential_comparisons = 0;
        long bst_comparisons = 0;
        long avl_comparisons = 0;

        int key = search_keys[i];

        found_sequential[i] =
            sequential_search(
                array,
                array_size,
                key,
                &sequential_comparisons
            );

        found_bst[i] =
            bst_search(
                bst_root,
                key,
                &bst_comparisons
            );

        found_avl[i] =
            avl_search(
                avl_root,
                key,
                &avl_comparisons
            );

        sequential_counts[i] =
            (int)sequential_comparisons;

        bst_counts[i] = bst_comparisons;
        avl_counts[i] = avl_comparisons;

        sequential_total += sequential_comparisons;
        bst_total += bst_comparisons;
        avl_total += avl_comparisons;

        printf(
            "%-4d %-8d %-8d %-8ld %-8ld %-15s %-15s %-15s\n",
            i + 1,
            key,
            sequential_counts[i],
            bst_counts[i],
            avl_counts[i],
            found_sequential[i] ? "Found" : "Not Found",
            found_bst[i] ? "Found" : "Not Found",
            found_avl[i] ? "Found" : "Not Found"
        );
    }

    printf("\n");

    printf("============================================================\n");
    printf("Search Summary\n");
    printf("============================================================\n");

    printf("Sequential Search\n");
    printf("  Total comparisons    : %ld\n",
           sequential_total);
    printf("  Average comparisons : %.2f\n",
           (double)sequential_total / NUM_SEARCH);

    printf("\n");

    printf("BST Search\n");
    printf("  Total comparisons    : %ld\n",
           bst_total);
    printf("  Average comparisons : %.2f\n",
           (double)bst_total / NUM_SEARCH);

    printf("\n");

    printf("AVL Search\n");
    printf("  Total comparisons    : %ld\n",
           avl_total);
    printf("  Average comparisons : %.2f\n",
           (double)avl_total / NUM_SEARCH);

    printf("\n");

    printf("============================================================\n");
    printf("Construction + Search Cost\n");
    printf("============================================================\n");

    printf("Array construction comparisons : %ld\n",
           array_build_comparisons);
    printf("Array search comparisons       : %ld\n",
           sequential_total);
    printf("Array total comparisons        : %ld\n",
           array_build_comparisons + sequential_total);

    printf("\n");

    printf("BST construction comparisons   : %ld\n",
           bst_build_comparisons);
    printf("BST search comparisons         : %ld\n",
           bst_total);
    printf("BST total comparisons          : %ld\n",
           bst_build_comparisons + bst_total);

    printf("\n");

    printf("AVL construction comparisons   : %ld\n",
           avl_build_comparisons);
    printf("AVL search comparisons         : %ld\n",
           avl_total);
    printf("AVL total comparisons          : %ld\n",
           avl_build_comparisons + avl_total);

    bst_free(bst_root);
    avl_free(avl_root);

    return 0;
}