#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 1000
#define QUEUE_SIZE 5000

struct node
{
    char data[20];
    struct node *left;
    struct node *right;
};

int total_nodes = 0;
int leaf_nodes = 0;
int max_degree = 0;

int left_used = 0;
int right_used = 0;
int not_skewed = 0;

char input[MAX_LEN];
int pos = 0;

void read_label(char label[])
{
    int i = 0;
    while (input[pos] != ',' && input[pos] != ')' && input[pos] != '\0')
    {
        label[i] = input[pos];
        i = i + 1;
        pos = pos + 1;
    }
    label[i] = '\0';
}

struct node *parse_node(int is_root)
{
    int has_paren = 0;
    char label[20];
    struct node *new_node;

    if (input[pos] == '(')
    {
        has_paren = 1;
        pos = pos + 1;
    }

    read_label(label);

    new_node = (struct node *)malloc(sizeof(struct node));
    strcpy(new_node->data, label);
    new_node->left = NULL;
    new_node->right = NULL;

    if (is_root == 1 && has_paren == 0 && input[pos] == '\0')
    {
        return new_node;
    }

    if (input[pos] == ',')
    {
        pos = pos + 1;
    }

    if (input[pos] == '(')
    {
        new_node->left = parse_node(0);
    }

    if (is_root == 1 && has_paren == 0 && input[pos] == '\0')
    {
        return new_node;
    }

    if (input[pos] == ',')
    {
        pos = pos + 1;
    }

    if (input[pos] == '(')
    {
        new_node->right = parse_node(0);
    }

    if (has_paren == 1)
    {
        pos = pos + 1;
    }

    return new_node;
}

void print_tree(struct node *cur, char prefix[], int is_last, int is_root)
{
    char new_prefix[200];

    if (cur == NULL)
    {
        return;
    }

    if (is_root == 1)
    {
        printf("%s\n", cur->data);
    }
    else
    {
        printf("%s+---%s\n", prefix, cur->data);
    }

    strcpy(new_prefix, prefix);
    if (is_root == 1)
    {
        new_prefix[0] = '\0';
    }
    else if (is_last == 1)
    {
        strcat(new_prefix, "    ");
    }
    else
    {
        strcat(new_prefix, "|   ");
    }

    if (cur->left != NULL)
    {
        print_tree(cur->left, new_prefix, cur->right == NULL, 0);
    }
    if (cur->right != NULL)
    {
        print_tree(cur->right, new_prefix, 1, 0);
    }
}

void compute_info(struct node *cur)
{
    int degree;

    if (cur == NULL)
    {
        return;
    }

    total_nodes = total_nodes + 1;

    degree = 0;
    if (cur->left != NULL)
    {
        degree = degree + 1;
    }
    if (cur->right != NULL)
    {
        degree = degree + 1;
    }

    if (degree > max_degree)
    {
        max_degree = degree;
    }
    if (degree == 0)
    {
        leaf_nodes = leaf_nodes + 1;
    }

    compute_info(cur->left);
    compute_info(cur->right);
}

int get_height(struct node *cur)
{
    int left_height, right_height;

    if (cur == NULL)
    {
        return 0;
    }

    left_height = get_height(cur->left);
    right_height = get_height(cur->right);

    if (left_height > right_height)
    {
        return left_height + 1;
    }
    else
    {
        return right_height + 1;
    }
}

int is_complete_tree(struct node *root)
{
    struct node *queue[QUEUE_SIZE];
    int front, rear;
    int seen_empty;
    struct node *cur;

    if (root == NULL)
    {
        return 1;
    }

    front = 0;
    rear = 0;
    seen_empty = 0;

    queue[rear] = root;
    rear = rear + 1;

    while (front < rear)
    {
        cur = queue[front];
        front = front + 1;

        if (cur == NULL)
        {
            seen_empty = 1;
        }
        else
        {
            if (seen_empty == 1)
            {
                return 0;
            }
            queue[rear] = cur->left;
            rear = rear + 1;
            queue[rear] = cur->right;
            rear = rear + 1;
        }
    }

    return 1;
}

int is_full_tree(int height)
{
    int full_count = 1;
    int i;
    for (i = 0; i < height; i++)
    {
        full_count = full_count * 2;
    }
    full_count = full_count - 1;

    if (total_nodes == full_count)
    {
        return 1;
    }
    return 0;
}

void check_skewed(struct node *cur)
{
    if (cur == NULL)
    {
        return;
    }

    if (cur->left != NULL && cur->right != NULL)
    {
        not_skewed = 1;
    }
    if (cur->left != NULL)
    {
        left_used = 1;
    }
    if (cur->right != NULL)
    {
        right_used = 1;
    }

    check_skewed(cur->left);
    check_skewed(cur->right);
}

int is_skewed_tree(struct node *root)
{
    left_used = 0;
    right_used = 0;
    not_skewed = 0;

    check_skewed(root);

    if (not_skewed == 1)
    {
        return 0;
    }
    if (left_used == 1 && right_used == 1)
    {
        return 0;
    }
    if (total_nodes > 1)
    {
        return 1;
    }
    return 0;
}

int main(void)
{
    char raw[MAX_LEN];
    char empty_prefix[200];
    int i, j;
    int height;
    struct node *root;

    printf("트리 괄호 표기법 입력: ");
    fgets(raw, MAX_LEN, stdin);

    j = 0;
    for (i = 0; raw[i] != '\0'; i++)
    {
        if (isspace((unsigned char)raw[i]) == 0)
        {
            input[j] = raw[i];
            j = j + 1;
        }
    }
    input[j] = '\0';

    pos = 0;
    root = parse_node(1);

    if (root == NULL)
    {
        printf("입력이 올바르지 않습니다.\n");
        return 1;
    }

    empty_prefix[0] = '\0';
    print_tree(root, empty_prefix, 1, 1);

    compute_info(root);
    height = get_height(root);

    //트리 정보 출력
    printf("전체 노드의 수    : %d\n", total_nodes);
    printf("단말 노드의 수    : %d\n", leaf_nodes);
    printf("비단말 노드의 수  : %d\n", total_nodes - leaf_nodes);
    printf("트리의 높이       : %d\n", height);
    printf("트리의 차수       : %d\n", max_degree);

    //이진트리 형태 판별
    if (is_complete_tree(root))
       printf("완전 이진트리 입니다.\n");
    if (is_full_tree(height))
        printf("포화 이진트리 입니다.\n");
    if (is_skewed_tree(root))
        printf("편향 이진트리 입니다.\n");

    return 0;
}