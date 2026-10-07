#include <stdio.h>
#include <string.h>

typedef struct
{
    char node_data[100];    // 스택에 쌓이는 영문 대문자 노드 문자들을 저장하는 배열
    int  node_top;                // node_data 배열의 최상단(Top) 인덱스 (초깃값: -1)
    int  counter_data[100]; // 괄호 중첩 단계별 자식 노드의 수(차수)를 기록하는 배열
    int  counter_top;             // counter_data 배열의 최상단(Top) 인덱스 (이 깊이가 트리의 높이가 됨)
} TreeStack;

void tree_stack_init(TreeStack *s)
{
    s->node_top = -1;
    s->counter_top = -1;
}

int node_is_empty(TreeStack *s)
{
    return s->node_top < 0;
}

void node_push(TreeStack *s, char node)
{
    s->node_top++;
    s->node_data[s->node_top] = node;
}

char node_pop(TreeStack *s)
{
    if (node_is_empty(s))
        return '\0';
    char popped = s->node_data[s->node_top];
    s->node_top--;
    return popped;
}

char node_peek(TreeStack *s)
{
    if (node_is_empty(s))
        return '\0';
    return s->node_data[s->node_top];
}

/* CounterStack 관련 메서드 */
int counter_is_empty(TreeStack *s) 
{
    return s->counter_top < 0;
}

void counter_push(TreeStack *s, int initial_count)
{
    s->counter_top++;
    s->counter_data[s->counter_top] = initial_count;
}

int counter_pop(TreeStack *s)
{
    if (counter_is_empty(s))
        return 0;
    int popped = s->counter_data[s->counter_top];
    s->counter_top--;
    return popped;
}

void counter_increment_top(TreeStack *s)
{
    if (counter_is_empty(s))
        return;
    s->counter_data[s->counter_top]++;
}

int counter_depth(TreeStack *s)
{
    return s->counter_top + 1;
}

/* 유효성 검사 함수: 입력된 문자열이 괄호 표기법 규칙에 맞는지 검증 */
int check_tree(const char *str)
{
    int len = strlen(str);
    if (len == 0)
        return 0;

    int bracket_count = 0;           // 괄호의 중첩 상태를 추적하는 카운터 (여는 괄호 +1, 닫는 괄호 -1)
    int expect_node = 1;             // 다음에 반드시 노드(알파벳 대문자)가 나와야 하는지 여부 (플래그)
    int expect_delim_or_bracket = 0; // 노드 직후이므로 콤마나 괄호가 와야 하는지 여부 (플래그)

    for (int i = 0; i < len; i++)
    {
        char c = str[i];
        if (c == ' ' || c == '\r' || c == '\n') continue;

        if (c >= 'A' && c <= 'Z')
        {
            if (!expect_node) return 0;
            expect_node = 0;
            expect_delim_or_bracket = 1;
        }
        else if (c == '(')
        {
            if (expect_node) return 0;
            bracket_count++;
            expect_node = 1;
            expect_delim_or_bracket = 0;
        }
        else if (c == ')')
        {
            bracket_count--;
            if (bracket_count < 0) return 0;
            expect_node = 0;
            expect_delim_or_bracket = 1;
        }
        else if (c == ',')
        {
            if (!expect_delim_or_bracket) return 0;
            expect_node = 1;
            expect_delim_or_bracket = 0;
        }
        else
            return 0;
    }

    if (bracket_count == 0 && expect_delim_or_bracket)
        return 1;
    return 0;
}

void print_tree(const char *str)
{
    int len = strlen(str);
    int depth = 0;
    int has_sibling[100] = {0}; //각 깊이별로 해당 레벨에 처리할 형제 노드가 남아있는지 기록하는 배열
    
    for (int i = 0; i < len; i++)
    {
        char c = str[i];
        if (c == ' ' || c == '\r' || c == '\n')
            continue;

        if (c >= 'A' && c <= 'Z')
        {
            if (depth == 0)
            {
                printf("%c\n", c);
            }
            else
            {
                for (int d = 0; d < depth - 1; d++)
                {
                    if (has_sibling[d])
                        printf("|   ");
                    else
                        printf("    ");
                }
                printf("+---%c\n", c);
            }
        }
        else if (c == '(')
        {
            depth++;
            int comma_count = 0;
            int bracket_level = 0;
            for (int j = i; j < len; j++)
            {
                if (str[j] == '(') bracket_level++;
                else if (str[j] == ')') bracket_level--;
                else if (str[j] == ',' && bracket_level == depth) comma_count++;
                if (bracket_level < depth - 1) break;
            }
            has_sibling[depth - 1] = (comma_count > 0) ? 1 : 0;
        }
        else if (c == ',')
        {
            has_sibling[depth - 1] = 1; 
        }
        else if (c == ')')
        {
            depth--;
            if (depth >= 0)
            {
                has_sibling[depth] = 0;
            }
        }
    }
}

int main(void) {
    char input[100];

    printf("트리 괄호 표기법 입력: ");
    if (fgets(input, sizeof(input), stdin) != NULL){
        input[strcspn(input, "\r\n")] = '\0';
    }

    if (!check_tree(input))
    {
        printf("오류: 올바른 트리의 괄호 표기법이 아닙니다.\n");
        return 1;
    }
    printf("유효한 트리 괄호 표기법입니다.\n");

    /* 통합 스택 선언 및 초기화 */
    TreeStack tstack;
    tree_stack_init(&tstack);

    int total_nodes = 0;    // 전체 노드(알파벳 대문자)의 총 개수
    int non_leaf_nodes = 0; // 비단말 노드(자식을 가지는 노드)의 총 개수
    int max_height = 0;     // 트리의 최대 높이(Height)
    int max_degree = 0;     // 트리의 최대 차수(Degree, 모든 노드의 자식 수 중 최댓값)

    char parent_of_C = '\0';         // 노드 C의 부모 노드 이름
    char children_of_C[50];          // 노드 C의 자식 노드들을 저장하는 배열
    int  child_count_C = 0;          // 노드 C가 가진 자식 노드의 개수
    int  capturing_children_of_C = 0;// 현재 C의 자식을 수집하는 구간인지 판별하는 플래그
    int  C_bracket_depth = -1;       // 노드 C가 속한 서브트리의 괄호 깊이 기준점

    char last_node = '\0';           // 직전에 읽은 노드가 무엇인지 기억하는 변수
    int len = strlen(input);

    /* 문자열을 순차적으로 스캔하며 각종 트리 정보 실시간 계산 */
    for (int i = 0; i < len; i++)
    {
        char c = input[i];
        if (c == ' ' || c == '\r' || c == '\n')
            continue;

        if (c >= 'A' && c <= 'Z') {
            total_nodes++;
            last_node = c;

            /* 현재 노드가 'C'이고 상위 노드가 존재한다면 직전 노드가 부모임 */
            if (!node_is_empty(&tstack) && c == 'C')
                parent_of_C = node_peek(&tstack);
            node_push(&tstack, c);

            /* 노드 C의 자식 수집 구간일 때, 정확히 직계 자식 레벨의 노드만 수집 */
            if (capturing_children_of_C && counter_depth(&tstack) == C_bracket_depth + 1)
                children_of_C[child_count_C++] = c;
        }
        else if (c == '(')
        {
            non_leaf_nodes++;
            counter_push(&tstack, 1);

            if (counter_depth(&tstack) > max_height)
                max_height = counter_depth(&tstack);
            
            /* 직전에 읽은 노드가 'C'였다면 자식 수집 시작 지점으로 설정 */
            if (last_node == 'C')
            {
                capturing_children_of_C = 1;
                C_bracket_depth = counter_depth(&tstack) - 1;
            }
            last_node = '\0';
        }
        else if (c == ',')
        {
            counter_increment_top(&tstack);
            /* 핵심: 형제 노드로 넘어갈 때 직전 형제 노드를 스택에서 제거하여 올바른 부모 관계 유지 */
            node_pop(&tstack);   
        }
        else if (c == ')')
        {
            int current_degree = counter_pop(&tstack);
            if (current_degree > max_degree) max_degree = current_degree;

            /* 노드 C의 자식 수집 범위가 끝나는 괄호 닫기 지점 감지 */
            if (capturing_children_of_C && counter_depth(&tstack) == C_bracket_depth)
                capturing_children_of_C = 0;
            node_pop(&tstack);
            last_node = '\0';
        }
    }

    int leaf_nodes = total_nodes - non_leaf_nodes; // 전체 노드 수에서 비단말 노드 수를 뺀 단말 노드 수
    children_of_C[child_count_C] = '\0';

    /* 결과 출력 */
    printf("\n=== 트리 정보 출력 ===\n");
    printf("- 전체 노드의 수: %d\n", total_nodes);
    printf("- 단말 노드의 수: %d\n", leaf_nodes);
    printf("- 비단말 노드의 수: %d\n", non_leaf_nodes);
    printf("- 트리의 높이: %d\n", max_height);
    printf("- 트리의 차수: %d\n", max_degree);

    if (parent_of_C != '\0') {
        printf("- 노드 C의 부모 노드: %c\n", parent_of_C);
    } else {
        printf("- 노드 C의 부모 노드: 없음\n");
    }

    printf("- 노드 C의 자식 노드: ");
    if (child_count_C > 0) {
        for (int i = 0; i < child_count_C; i++) printf("%c ", children_of_C[i]);
        printf("\n");
    } else {
        printf("없음\n");
    }

    print_tree(input);

    return 0;
}