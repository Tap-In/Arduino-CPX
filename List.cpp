#include <stdio.h>
#include <stdlib.h>
#include "List.h"

/* Create the list */
List *List_create()
{
    return (List*)calloc(1, sizeof(List));
}

/* Destroys a List and any elements it might have. */
void List_destroy(List *list)
{
    LIST_FOREACH(list, first, next, cur)
    {
        if(cur->prev)
        {
            free(cur->prev);
        }
    }

    free(list->last);
    free(list);
}

/* Convenience function for freeing the values in each node, not the nodes. */
void List_clear(List *list)
{
    LIST_FOREACH(list, first, next, cur)
    {
        free((void*)cur->value);
    }
}

/* Clears and destroys a list. It's not very efficient since it loops through them twice. */
void List_clear_destroy(List *list)
{
    List_clear(list);
    List_destroy(list);
}

/* It adds a new element to the end of the list, and because that's just a couple of pointer assignments, does it very fast. */
void List_push(List *list, const void *value)
{
    ListNode *node = (ListNode*)calloc(1, sizeof(ListNode));

    node->value = value;

    if(list->last == NULL)
    {
        list->first = node;
        list->last = node;
    }
    else
    {
        list->last->next = node;
        node->prev = list->last;
        list->last = node;
    }

    list->count++;

    return;
}

/* The inverse of List_push, this takes the last element off and returns it. */
const void *List_pop(List *list)
{
    ListNode *node = list->last;
    return node != NULL ? List_remove(list, node) : NULL;
}

/* add elements to the front of the list very fast. In this case I call that List_unshift for lack of a better term. */
void List_unshift(List *list, void *value)
{
    ListNode *node = (ListNode*)calloc(1, sizeof(ListNode));

    node->value = value;

    if(list->first == NULL)
    {
        list->first = node;
        list->last = node;
    }
    else
    {
        node->next = list->first;
        list->first->prev = node;
        list->first = node;
    }

    list->count++;

    return;
}

/* Just like List_pop, this removes the first element and returns it. */
const void *List_shift(List *list)
{
    ListNode *node = list->first;
    return node != NULL ? List_remove(list, node) : NULL;
}

/* This is actually doing all of the removal when you do List_pop or List_shift. Something that seems to always be difficult in data structures is removing things,
and this function is no different. It has to handle quite a few conditions depending on if the element being removed is at the front; the end; both front and
end; or middle. */

const void *List_remove(List *list, ListNode *node)
{
    const void *result = NULL;

    //check(list->first && list->last, "List is empty.");
    //check(node, "node can't be NULL");

    if(node == list->first && node == list->last)
    {
        list->first = NULL;
        list->last = NULL;
    }
    else if(node == list->first)
    {
        list->first = node->next;
        // check(list->first != NULL, "Invalid list, somehow got a first that is NULL.");
        list->first->prev = NULL;
    }
    else if (node == list->last)
    {
        list->last = node->prev;
        //check(list->last != NULL, "Invalid list, somehow got a next that is NULL.");
        list->last->next = NULL;
    }
    else
    {
        ListNode *after = node->next;
        ListNode *before = node->prev;
        after->prev = before;
        before->next = after;
    }

    list->count--;
    result = node->value;
    free(node);

    return result;
}

/* Return value at index */
const void* List_query(List *list, int rc)
{
    int i = 0;
    if (rc >= list->count)
        return NULL;

    ListNode* node = list->first;
    for(i=0; i<rc; i++)
    {
        node = node->next;
    }
    return node->value;
}

void List_delete(List *list,int rc)
{
    int i = 0;
    if (rc >= list->count)
        return;

    ListNode* node = list->first;
    for(i=0; i<rc; i++)
    {
        node = node->next;
    }
    List_remove(list,node);
}

/* Clip a node in the middule of the list at index rc */
const void* List_clip(List* list, int rc)
{
    int i = 0;
    if (rc >= list->count)
        return NULL;

    ListNode* node = list->first;
    for(i=0; i<rc; i++)
    {
        node = node->next;
    }
    if (node != NULL)
        return List_remove(list,node);
    else
        return NULL;
}

void List_paste(List* list, int rc, void* value)
{
    int i = 0;
    ListNode *newNode;

    if (rc >= list->count)
    {
        List_push(list,value);
        return;
    }

    ListNode* node = list->first;
    for(i=0; i<rc; i++)
    {
        node = node->next;
    }
    if (node->prev == NULL)
    {
        List_unshift(list,value);
        return;
    }
    node = node->prev;
    newNode = (ListNode*)calloc(1, sizeof(ListNode));
    newNode->value = value;
    newNode->next = node->next;
    node->next = newNode;
    newNode->prev = node;
    list->count++;
}

int List_size(List* list)
{
    return list->count;
}

