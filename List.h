#pragma once
#include <stdlib.h>

struct ListNode;

typedef struct ListNode
{
    struct ListNode *next;
    struct ListNode *prev;
    const void *value;
} ListNode;

typedef struct List
{
    int count;
    ListNode *first;
    ListNode *last;
} List;

List *List_create();
void List_destroy(List *list);
void List_clear(List *list);
void List_clear_destroy(List *list);

#define List_count(A) ((A)->count)
#define List_first(A) ((A)->first != NULL ? (A)->first->value : NULL)
#define List_last(A) ((A)->last != NULL ? (A)->last->value : NULL)

void List_push(List *list, const void *value);
const void *List_pop(List *list);

void List_unshift(List *list, void *value);
const void *List_shift(List *list);

const void *List_remove(List *list, ListNode *node);

const void* List_query(List *list, int rc);
const void* List_clip(List* list, int rc);
void List_paste(List* list, int rc, void* value);
int List_size(List* list) ;

void List_delete(List *list,int rc);

#define LIST_FOREACH(L, S, M, V) ListNode *_node = NULL;\
    ListNode *V = NULL;\
    for(V = _node = L->S; _node != NULL; V = _node = _node->M)
