#include <stdio.h>
#include <stdlib.h>

#define MAX_ELEMENT 100

typedef struct s_heap
{
    int heap[MAX_ELEMENT];
    int heap_size;
} t_heap;

t_heap *createHeap()
{
    t_heap *h = (t_heap *)malloc(sizeof(t_heap));
    h->heap_size = 0;
    return h;
}

int isEmpty(t_heap *h)
{
    return h->heap_size == 0;
}

void insertHeap(t_heap *h, int item)
{
    int i = ++(h->heap_size);
    while (i != 1 && item > h->heap[i / 2])
    {
        h->heap[i] = h->heap[i / 2];
        i /= 2;
    }
    h->heap[i] = item;
}

int deleteHeap(t_heap *h)
{
    if (isEmpty(h))
        return 0;
    int item = h->heap[1];
    int temp = h->heap[h->heap_size--];
    int parent = 1, child = 2;
    while (child <= h->heap_size) {
        if (child < h->heap_size && h->heap[child] < h->heap[child + 1])
            child++;
        if (temp >= h->heap[child]) break;
        h->heap[parent] = h->heap[child];
        parent = child;
        child *= 2;
    }
    h->heap[parent] = temp;
    return item;
}

void printHeap(t_heap* h) {
    int i;
    printf("Heap : ");
    for (i = 1; i <= h->heap_size; i++)
        printf("[%d] ", h->heap[i]);
    printf("\n");
}
 
/* 결과가 정말 최대 히프인지 검사: 모든 i에 대해 부모 >= 자식 */
int isMaxHeap(t_heap* h) {
    int i;
    for (i = 2; i <= h->heap_size; i++)
        if (h->heap[i / 2] < h->heap[i]) return 0;
    return 1;
}
 
int main(void) {
    int i, n, k;
 
    /* 테스트 1: 교재 예제 7-6 */
    printf("== 테스트 1: 예제 7-6 (10, 45, 19, 11, 96) ==\n");
    t_heap* heap = createHeap();
    printf("isEmpty = %d\n", isEmpty(heap));            // 1
    insertHeap(heap, 10);
    insertHeap(heap, 45);
    insertHeap(heap, 19);
    insertHeap(heap, 11);
    insertHeap(heap, 96);
    printHeap(heap);                                     // [96] [45] [19] [10] [11]
    n = heap->heap_size;
    for (i = 1; i <= n; i++)
        printf(" delete : [%d]\n", deleteHeap(heap));   // 96 45 19 11 10
    printf("isEmpty = %d\n", isEmpty(heap));            // 1
    deleteHeap(heap);                                    // Heap is empty
 
    /* 테스트 2: 슬라이드 삽입 예 (17, 23) */
    printf("\n== 테스트 2: 삽입 예 ==\n");
    int base[] = {20, 15, 19, 8, 13, 10};
    t_heap* h2 = createHeap();
    for (k = 0; k < 6; k++) insertHeap(h2, base[k]);
    printHeap(h2);                                       // [20] [15] [19] [8] [13] [10]
    insertHeap(h2, 17);
    printf("+17 -> "); printHeap(h2);                    // 17은 7번 자리에서 확정
    t_heap* h3 = createHeap();
    for (k = 0; k < 6; k++) insertHeap(h3, base[k]);
    insertHeap(h3, 23);
    printf("+23 -> "); printHeap(h3);                    // [23] [15] [20] [8] [13] [10] [19]
 
    /* 테스트 3: 슬라이드 삭제 예 */
    printf("\n== 테스트 3: 삭제 예 ==\n");
    int base2[] = {20, 19, 15, 8, 13, 10};
    t_heap* h4 = createHeap();
    for (k = 0; k < 6; k++) insertHeap(h4, base2[k]);
    printHeap(h4);                                       // [20] [19] [15] [8] [13] [10]
    printf("delete : [%d]\n", deleteHeap(h4));          // 20
    printHeap(h4);                                       // [19] [13] [15] [8] [10]
 
    /* 테스트 4: 무작위 100번 반복, 히프 조건과 내림차순 확인 */
    printf("\n== 테스트 4: 무작위 검사 ==\n");
    srand(42);
    int ok = 1;
    for (int t = 0; t < 100; t++) {
        t_heap* r = createHeap();
        int cnt = rand() % 50 + 1;
        for (k = 0; k < cnt; k++) {
            insertHeap(r, rand() % 1000);
            if (!isMaxHeap(r)) ok = 0;
        }
        int prev = 1000;
        while (!isEmpty(r)) {
            int x = deleteHeap(r);
            if (x > prev || !isMaxHeap(r)) ok = 0;   // 항상 내림차순으로 나와야 함
            prev = x;
        }
        free(r);
    }
    printf("%s\n", ok ? "PASS: 100번 모두 히프 조건 + 내림차순 OK" : "FAIL");
 
    free(heap); free(h2); free(h3); free(h4);
    return 0;
}