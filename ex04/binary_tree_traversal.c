#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 1000
#define STACK_SIZE 1024

struct node
{
    char data[20];      // 노드에 저장될 문자열 데이터
    struct node *left;  // 왼쪽 자식 노드 포인터
    struct node *right; // 오른쪽 자식 노드 포인터
};

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

// 괄호와 쉼표로 표현된 문자열을 재귀적으로 파싱하여 이진트리를 구축하는 함수
struct node *parse_node(int is_root)
{
    int has_paren = 0;
    char label[20];
    struct node *new_node;

    // 시작이 괄호인 경우 처리
    if (input[pos] == '(')
    {
        has_paren = 1;
        pos = pos + 1;
    }

    if (input[pos] == ',' || input[pos] == ')' || input[pos] == '\0')
    {
        printf("ERROR: 잘못된 입력 형식입니다.\n");
        exit(1);
    }

    read_label(label);

    // 새 노드 동적 할당 및 초기화
    new_node = (struct node *)malloc(sizeof(struct node));
    strcpy(new_node->data, label);
    new_node->left = NULL;
    new_node->right = NULL;

    if (is_root == 1 && has_paren == 0 && input[pos] == '\0')
    {
        return new_node;
    }

    if (input[pos] != ',')
    {
        printf("ERROR: 잘못된 입력 형식입니다.\n");
        exit(1);
    }
    pos = pos + 1;

    // 왼쪽 자식 파싱 (존재하는 경우)
    if (input[pos] == '(')
    {
        new_node->left = parse_node(0);
    }

    if (is_root == 1 && has_paren == 0 && input[pos] == '\0')
    {
        return new_node;
    }

    if (input[pos] != ',')
    {
        printf("ERROR: 잘못된 입력 형식입니다.\n");
        exit(1);
    }
    pos = pos + 1;

    // 오른쪽 자식 파싱 (존재하는 경우)
    if (input[pos] == '(')
    {
        new_node->right = parse_node(0);
    }

    // 닫는 괄호 처리
    if (has_paren == 1)
    {
        if (input[pos] != ')')
        {
            printf("ERROR: 잘못된 입력 형식입니다.\n");
            exit(1);
        }
        pos = pos + 1;
    }

    return new_node;
}

// 스택을 이용한 전위 순회 출력
void preorder(struct node *tree)
{
    struct node *stack[STACK_SIZE];
    int top = -1;
    struct node *cur;

    printf("Preorder  : ");

    if (tree != NULL)
    {
        top = top + 1;
        stack[top] = tree;
    }

    while (top >= 0)
    {
        cur = stack[top];
        top = top - 1;

        printf("%s ", cur->data);

        // 스택의 특성상 오른쪽 자식을 먼저 넣어야 왼쪽 자식이 먼저 출력됨
        if (cur->right != NULL)
        {
            top = top + 1;
            stack[top] = cur->right;
        }
        if (cur->left != NULL)
        {
            top = top + 1;
            stack[top] = cur->left;
        }
    }

    printf("\n");
}

// 스택을 이용한 중위 순회 출력
void inorder(struct node *tree)
{
    struct node *stack[STACK_SIZE];
    int top = -1;
    struct node *cur = tree;

    printf("Inorder   : ");

    while (cur != NULL || top >= 0)
    {
        while (cur != NULL)
        {
            top = top + 1;
            stack[top] = cur;
            cur = cur->left;
        }

        cur = stack[top];
        top = top - 1;

        printf("%s ", cur->data);

        cur = cur->right;
    }

    printf("\n");
}

// 두 개의 스택을 이용한 후위 순회 출력
void postorder(struct node *tree)
{
    struct node *stack1[STACK_SIZE];
    struct node *stack2[STACK_SIZE];
    int top1 = -1;
    int top2 = -1;
    struct node *cur;

    if (tree != NULL)
    {
        top1 = top1 + 1;
        stack1[top1] = tree;
    }

    while (top1 >= 0)
    {
        cur = stack1[top1];
        top1 = top1 - 1;

        top2 = top2 + 1;
        stack2[top2] = cur;

        if (cur->left != NULL)
        {
            top1 = top1 + 1;
            stack1[top1] = cur->left;
        }
        if (cur->right != NULL)
        {
            top1 = top1 + 1;
            stack1[top1] = cur->right;
        }
    }

    printf("Postorder : ");

    while (top2 >= 0)
    {
        cur = stack2[top2];
        top2 = top2 - 1;
        printf("%s ", cur->data);
    }

    printf("\n");
}

// 이진트리를 왼쪽으로 눕힌 형태로 시각화하여 출력하는 함수
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

int main(void)
{
    // 괄호 표기법으로 표현된 이진트리 입력 문자열
    char raw[MAX_LEN] = "A,(B,(D,(H,,),(I,,)),(E,(K,,),(L,,))),(C,(F,(J,,),(K,,)),(G,,))";
    char empty_prefix[200];
    int i, j;
    struct node *root;
    
    // 입력 문자열에서 공백 제거 후 input 배열에 저장
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

    if (j == 0)
    {
        printf("오류: 입력이 비어 있습니다.\n");
        return 1;
    }

    pos = 0;
    root = parse_node(1);

    printf("=== 시험 결과 ===\n");
    printf("\n입력된 이진트리 구조\n");
    empty_prefix[0] = '\0';
    print_tree(root, empty_prefix, 1, 1);

    printf("\n");
    preorder(root);
    inorder(root);
    postorder(root);

    return 0;
}