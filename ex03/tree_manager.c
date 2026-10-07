#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 200
#define MAX_PATH 200
#define MAX_TOKENS 10

// 이진트리 노드 구조체
struct node
{
    char data;          // 노드에 저장된 데이터 (영문 대문자)
    struct node *left;  // 왼쪽 자식 노드 포인터
    struct node *right; // 오른쪽 자식 노드 포인터
};

// 이진트리 전체 관리 구조체
typedef struct
{
    struct node *root;  // 루트 노드 포인터
    int max_size;       // 최대 저장 가능한 노드 수
    int count;          // 현재 저장된 노드 수
} tree_t;

// 경로(/A/B/C)를 따라가며 해당하는 노드를 찾아 반환하는 함수
struct node *find_node(struct node *root, char path[])
{
    char buf[MAX_PATH];
    char *token;
    struct node *cur;

    if (strcmp(path, "/") == 0)
        return NULL;

    strcpy(buf, path);
    token = strtok(buf + 1, "/");

    if (token == NULL || root == NULL || strlen(token) != 1 || root->data != token[0])
        return NULL;

    cur = root;
    token = strtok(NULL, "/");

    while (token != NULL)
    {
        if (strlen(token) != 1)
            return NULL;
        if (cur->left != NULL && cur->left->data == token[0])
            cur = cur->left;
        else if (cur->right != NULL && cur->right->data == token[0])
            cur = cur->right;
        else
            return NULL;
        token = strtok(NULL, "/");
    }

    return cur;
}

// 특정 경로의 노드에 대한 부모 노드를 찾아 반환하는 함수
struct node *find_parent(struct node *root, char path[])
{
    char buf[MAX_PATH];
    char *token;
    struct node *cur;
    struct node *parent;

    if (strcmp(path, "/") == 0)
        return NULL;

    strcpy(buf, path);
    token = strtok(buf + 1, "/");

    if (token == NULL || root == NULL || strlen(token) != 1 || root->data != token[0])
        return NULL;

    cur = root;
    parent = NULL;
    token = strtok(NULL, "/");

    while (token != NULL)
    {
        if (strlen(token) != 1)
            return NULL;
        
        if (cur->left != NULL && cur->left->data == token[0])
        {
            parent = cur;
            cur = cur->left;
        }
        else if (cur->right != NULL && cur->right->data == token[0])
        {
            parent = cur;
            cur = cur->right;
        }
        else
            return NULL;
        token = strtok(NULL, "/");
    }

    return parent;
}

// 이진트리를 왼쪽으로 눕힌 형태로 재귀 출력하는 보조 함수
void print_node(struct node *cur, char prefix[], int is_last, int is_root)
{
    char new_prefix[MAX_PATH];

    if (cur == NULL)
        return;

    if (is_root == 1)
        printf("%c\n", cur->data);
    else
        printf("%s+---%c\n", prefix, cur->data);

    strcpy(new_prefix, prefix);
    if (is_root == 1)
        new_prefix[0] = '\0';
    else if (is_last == 1)
        strcat(new_prefix, "    ");
    else
        strcat(new_prefix, "|   ");

    if (cur->left != NULL)
        print_node(cur->left, new_prefix, cur->right == NULL, 0);
    if (cur->right != NULL)
        print_node(cur->right, new_prefix, 1, 0);
}

void free_tree(struct node *cur)
{
    if (cur == NULL)
        return;
    free_tree(cur->left);
    free_tree(cur->right);
    free(cur);
}

tree_t create_btree(int size)
{
    tree_t t;
    t.root = NULL;
    t.max_size = size;
    t.count = 0;
    return t;
}

tree_t insert_root(tree_t t, char value)
{
    if (t.root != NULL)
    {
        printf("오류: 이미 루트 노드가 존재합니다.\n");
        return t;
    }
    if (t.count >= t.max_size)
    {
        printf("오류: 트리 크기가 가득 찼습니다.\n");
        return t;
    }

    t.root = (struct node *)malloc(sizeof(struct node));
    t.root->data = value;
    t.root->left = NULL;
    t.root->right = NULL;
    t.count = t.count + 1;

    return t;
}

tree_t insert_child(tree_t t, char parent_path[], char child, char value)
{
    struct node *parent;
    struct node *sibling;
    struct node *new_node;

    if (t.count >= t.max_size)
    {
        printf("오류: 트리 크기가 가득 찼습니다.\n");
        return t;
    }

    parent = find_node(t.root, parent_path);
    if (parent == NULL)
    {
        printf("오류: 존재하지 않는 노드 경로입니다.\n");
        return t;
    }

    if (child != 'L' && child != 'R')
    {
        printf("오류: child는 L 또는 R이어야 합니다.\n");
        return t;
    }

    if (parent->left != NULL && parent->right != NULL)
    {
        printf("오류: 부모 노드에 이미 자식이 2개 있습니다.\n");
        return t;
    }

    if (child == 'L')
    {
        if (parent->left != NULL)
        {
            printf("오류: 왼쪽 자식이 이미 존재합니다.\n");
            return t;
        }
        sibling = parent->right;
    }
    else
    {
        if (parent->right != NULL)
        {
            printf("오류: 오른쪽 자식이 이미 존재합니다.\n");
            return t;
        }
        sibling = parent->left;
    }

    if (sibling != NULL && sibling->data == value)
    {
        printf("오류: 같은 부모의 다른 자식과 데이터가 동일합니다.\n");
        return t;
    }

    new_node = (struct node *)malloc(sizeof(struct node));
    new_node->data = value;
    new_node->left = NULL;
    new_node->right = NULL;

    if (child == 'L')
        parent->left = new_node;
    else
        parent->right = new_node;

    t.count = t.count + 1;
    return t;
}

tree_t delete_node(tree_t t, char leaf_path[])
{
    struct node *target;
    struct node *parent;

    target = find_node(t.root, leaf_path);
    if (target == NULL)
    {
        printf("오류: 존재하지 않는 노드 경로입니다.\n");
        return t;
    }

    if (target->left != NULL || target->right != NULL)
    {
        printf("오류: 단말 노드가 아닙니다.\n");
        return t;
    }

    parent = find_parent(t.root, leaf_path);

    if (parent == NULL)
        t.root = NULL;
    else if (parent->left == target)
        parent->left = NULL;
    else
        parent->right = NULL;

    free(target);
    t.count = t.count - 1;
    return t;
}

tree_t update_value(tree_t t, char node_path[], char value)
{
    struct node *target;
    struct node *parent;
    struct node *sibling;

    target = find_node(t.root, node_path);
    if (target == NULL)
    {
        printf("오류: 존재하지 않는 노드 경로입니다.\n");
        return t;
    }

    parent = find_parent(t.root, node_path);
    sibling = NULL;
    if (parent != NULL)
    {
        if (parent->left == target)
            sibling = parent->right;
        else
            sibling = parent->left;
    }

    if (sibling != NULL && sibling->data == value)
    {
        printf("오류: 같은 부모의 다른 자식과 데이터가 동일합니다.\n");
        return t;
    }

    target->data = value;
    return t;
}

void read_child(tree_t t, char parent_path[])
{
    struct node *target = find_node(t.root, parent_path);

    if (target == NULL)
    {
        printf("오류: 존재하지 않는 노드 경로입니다.\n");
        return;
    }

    if (target->left != NULL && target->right != NULL)
        printf("%c(L), %c(R)\n", target->left->data, target->right->data);
    else if (target->left != NULL)
        printf("%c(L)\n", target->left->data);
    else if (target->right != NULL)
        printf("%c(R)\n", target->right->data);
    else
        printf("자식이 없습니다.\n");
}

void print_btree(tree_t t)
{
    char empty_prefix[MAX_PATH];

    if (t.root == NULL)
    {
        printf("트리가 비어 있습니다.\n");
        return;
    }

    empty_prefix[0] = '\0';
    print_node(t.root, empty_prefix, 1, 1);
}

tree_t destroy_btree(tree_t t)
{
    free_tree(t.root);
    t.root = NULL;
    t.count = 0;
    return t;
}

// 명령어 전체 이름 또는 첫 글자 매칭 검사 함수
int match_command(char token[], char full_name[])
{
    if (isupper((unsigned char)token[0]) == 0)
        return 0;
    if (strcmp(token, full_name) == 0)
        return 1;
    if (strlen(token) == 1 && token[0] == full_name[0])
        return 1;
    return 0;
}

// 입력된 문자열이 유효한 영문 대문자 한 글자 인지 확인
int is_upper_letter(char s[])
{
    if (strlen(s) != 1)
        return 0;
    if (isupper((unsigned char)s[0]) == 0)
        return 0;
    return 1;
}

tree_t process_insert(tree_t t, char tokens[][MAX_LINE], int count)
{
    if (count == 3) // 루트 노드 생성인 경우 (Insert / A)
    {
        if (strcmp(tokens[1], "/") != 0)
        {
            printf("오류: 존재하지 않는 노드 경로입니다.\n");
            return t;
        }
        if (is_upper_letter(tokens[2]) == 0)
        {
            printf("오류: 데이터는 영문 대문자 한 글자여야 합니다.\n");
            return t;
        }
        return insert_root(t, tokens[2][0]);
    }

    if (count == 4) // 자식 노드 추가인 경우 (Insert parent child data)
    {
        if (strcmp(tokens[1], "/") == 0)
        {
            printf("오류: 존재하지 않는 노드 경로입니다.\n");
            return t;
        }
        if (strlen(tokens[2]) != 1 || (toupper((unsigned char)tokens[2][0]) != 'L' && toupper((unsigned char)tokens[2][0]) != 'R'))
        {
            printf("오류: child는 L 또는 R이어야 합니다.\n");
            return t;
        }
        if (is_upper_letter(tokens[3]) == 0)
        {
            printf("오류: 데이터는 영문 대문자 한 글자여야 합니다.\n");
            return t;
        }
        return insert_child(t, tokens[1], toupper((unsigned char)tokens[2][0]), tokens[3][0]);
    }

    printf("오류: 인자의 개수가 맞지 않습니다.\n");
    return t;
}

tree_t process_delete(tree_t t, char tokens[][MAX_LINE], int count)
{
    if (count != 2)
    {
        printf("오류: 인자의 개수가 맞지 않습니다.\n");
        return t;
    }
    if (strcmp(tokens[1], "/") == 0)
    {
        printf("오류: 존재하지 않는 노드 경로입니다.\n");
        return t;
    }
    return delete_node(t, tokens[1]);
}

tree_t process_update(tree_t t, char tokens[][MAX_LINE], int count)
{
    if (count != 3)
    {
        printf("오류: 인자의 개수가 맞지 않습니다.\n");
        return t;
    }
    if (is_upper_letter(tokens[2]) == 0)
    {
        printf("오류: 데이터는 영문 대문자 한 글자여야 합니다.\n");
        return t;
    }
    return update_value(t, tokens[1], tokens[2][0]);
}

void process_read(tree_t t, char tokens[][MAX_LINE], int count)
{
    if (count != 2)
    {
        printf("오류: 인자의 개수가 맞지 않습니다.\n");
        return;
    }
    read_child(t, tokens[1]);
}

void process_print(tree_t t, char tokens[][MAX_LINE], int count)
{
    if (count != 1)
    {
        printf("오류: 인자의 개수가 맞지 않습니다.\n");
        return;
    }
    print_btree(t);
}

// 메인 함수: 사용자 입력을 받아 명령어를 분기 처리
int main(void)
{
    tree_t t;
    char line[MAX_LINE];
    char tokens[MAX_TOKENS][MAX_LINE];
    int count;
    char *tok;

    // 최대 1000개 노드를 저장할 수 있는 빈 이진트리 생성
    t = create_btree(1000);

    // 표준 입력으로부터 한 줄씩 명령어를 읽어 처리
    while (fgets(line, MAX_LINE, stdin) != NULL)
    {
        count = 0;
        tok = strtok(line, " \t\r\n");
        while (tok != NULL && count < MAX_TOKENS)
        {
            strcpy(tokens[count], tok);
            count = count + 1;
            tok = strtok(NULL, " \t\r\n");
        }

        if (count == 0)
            continue;

        // 명령어 매칭 및 실행
        if (match_command(tokens[0], "Insert") == 1)
            t = process_insert(t, tokens, count);
        else if (match_command(tokens[0], "Delete") == 1)
            t = process_delete(t, tokens, count);
        else if (match_command(tokens[0], "Update") == 1)
            t = process_update(t, tokens, count);
        else if (match_command(tokens[0], "Read") == 1)
            process_read(t, tokens, count);
        else if (match_command(tokens[0], "Print") == 1)
            process_print(t, tokens, count);
        else
            printf("오류: 잘못된 명령어입니다.\n");
    }

    // 프로그램 종료 전 동적 할당된 트리 메모리 해제
    t = destroy_btree(t);
    return 0;
}
