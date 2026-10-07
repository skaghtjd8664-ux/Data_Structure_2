#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NODES 1024
#define MAX_LEN 1000

char node_data[MAX_NODES][20];
int is_used[MAX_NODES];
int max_index = 0;

int total_nodes = 0;
int leaf_nodes = 0;
int max_degree = 0;

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
}

void parse_node(int index, int is_root)
{
    int has_paren = 0;
    char label[20];

    if (index >= MAX_NODES)
    {
        printf("오류: 트리가 너무 큽니다.\n");
        exit(1);
    }

    if (input[pos] == '(')
    {
        has_paren = 1;
        pos = pos + 1;
    }

    read_label(label);
    strcpy(node_data[index], label);
    is_used[index] = 1;
    if (index > max_index)
    {
        max_index = index;
    }

    if (is_root == 1 && has_paren == 0 && input[pos] == '\0')
    {
        return;
    }

    if (input[pos] == ',')
    {
        pos = pos + 1;
    }

    if (input[pos] == '(')
    {
        parse_node(index * 2, 0);
    }

    if (is_root == 1 && has_paren == 0 && input[pos] == '\0')
    {
        return;
    }

    if (input[pos] == ',')
    {
        pos = pos + 1;
    }

    if (input[pos] == '(')
    {
        parse_node(index * 2 + 1, 0);
    }

    if (has_paren == 1)
    {
        pos = pos + 1;
    }
}

int has_left_child(int index)
{
    int left = index * 2;
    if (left <= max_index && is_used[left] == 1)
    {
        return 1;
    }
    return 0;
}

int has_right_child(int index)
{
    int right = index * 2 + 1;
    if (right <= max_index && is_used[right] == 1)
    {
        return 1;
    }
    return 0;
}

void print_tree(int index, char prefix[], int is_last, int is_root)
{
    char new_prefix[200];

    if (index > max_index || is_used[index] == 0)
    {
        return;
    }

    if (is_root == 1)
    {
        printf("%s\n", node_data[index]);
    }
    else
    {
        printf("%s+---%s\n", prefix, node_data[index]);
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

    if (has_left_child(index) == 1)
    {
        print_tree(index * 2, new_prefix, !has_right_child(index), 0);
    }
    if (has_right_child(index) == 1)
    {
        print_tree(index * 2 + 1, new_prefix, 1, 0);
    }
}

void compute_info(void)
{
    int i;
    total_nodes = 0;
    leaf_nodes = 0;
    max_degree = 0;

    for (i = 1; i <= max_index; i++)
    {
        if (is_used[i] == 0)
        {
            continue;
        }

        total_nodes = total_nodes + 1;

        int degree = 0;
        if (has_left_child(i) == 1)
        {
            degree = degree + 1;
        }
        if (has_right_child(i) == 1)
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
    }
}

int get_height(int index)
{
    int left_height, right_height;

    if (index > max_index || is_used[index] == 0)
    {
        return 0;
    }

    left_height = get_height(index * 2);
    right_height = get_height(index * 2 + 1);

    if (left_height > right_height)
    {
        return left_height + 1;
    }
    else
    {
        return right_height + 1;
    }
}

int is_complete_tree(void)
{
    int i;
    for (i = 1; i <= total_nodes; i++)
    {
        if (is_used[i] == 0)
        {
            return 0;
        }
    }
    for (i = total_nodes + 1; i <= max_index; i++)
    {
        if (is_used[i] == 1)
        {
            return 0;
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

int is_skewed_tree(void)
{
    int i;
    int left_used = 0;
    int right_used = 0;

    for (i = 1; i <= max_index; i++)
    {
        if (is_used[i] == 0)
        {
            continue;
        }
        if (has_left_child(i) == 1 && has_right_child(i) == 1)
        {
            return 0;
        }
        if (has_left_child(i) == 1)
        {
            left_used = 1;
        }
        if (has_right_child(i) == 1)
        {
            right_used = 1;
        }
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
    parse_node(1, 1);

    if (is_used[1] == 0)
    {
        printf("입력이 올바르지 않습니다.\n");
        return 1;
    }

    empty_prefix[0] = '\0';
    print_tree(1, empty_prefix, 1, 1);

    compute_info();
    height = get_height(1);

    //트리 정보 출력
    printf("전체 노드의 수    : %d\n", total_nodes);
    printf("단말 노드의 수    : %d\n", leaf_nodes);
    printf("비단말 노드의 수  : %d\n", total_nodes - leaf_nodes);
    printf("트리의 높이       : %d\n", height);
    printf("트리의 차수       : %d\n", max_degree);

    //이진트리 형태 판별
    if (is_complete_tree())
       printf("완전 이진트리 입니다.\n");
    if (is_full_tree(height))
        printf("포화 이진트리 입니다.\n");
    if (is_skewed_tree())
        printf("편향 이진트리 입니다.\n");

    return 0;
}