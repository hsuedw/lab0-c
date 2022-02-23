#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    element_t *q = malloc(sizeof(element_t));
    if (!q)
        return NULL;

    q->value = NULL;
    INIT_LIST_HEAD(&q->list);

    return &q->list;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;

    struct list_head *it = l->next;
    while (it != l) {
        element_t *e = container_of(it, element_t, list);
        it = it->next;
        q_release_element(e);
    }
    element_t *head = container_of(l, element_t, list);
    free(head);
}

/*
 * q_init_element() initializes a new object of element_t.
 * Assume e and s are not NULL and point to valid memory address.
 * Return true if successful. Otherwise, return false.
 */
bool q_init_element(element_t *e, char *s)
{
    size_t len = strlen(s) + 1;
    e->value = malloc(len);
    if (!e->value)
        return false;

    memcpy(e->value, s, len);
    INIT_LIST_HEAD(&e->list);

    return true;
}

/*
 * q_new_element() create a new object of element_t.
 * Return NULL if could not allocate space.
 */
element_t *q_new_element(char *s)
{
    element_t *ne = malloc(sizeof(element_t));
    if (!ne)
        return NULL;

    if (!q_init_element(ne, s)) {
        free(ne);
        return NULL;
    }

    return ne;
}


/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ne = q_new_element(s);
    if (!ne)
        return false;

    list_add(&ne->list, head);

    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *ne = q_new_element(s);
    if (!ne)
        return false;

    list_add_tail(&ne->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head) || !sp)
        return NULL;

    element_t *target = list_entry(head->next, element_t, list);
    list_del_init(head->next);
    strncpy(sp, target->value, bufsize - 1);

    sp[bufsize - 1] = '\0';
    return target;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head) || !sp)
        return NULL;

    element_t *target = list_entry(head->prev, element_t, list);
    list_del_init(head->prev);
    strncpy(sp, target->value, bufsize - 1);

    sp[bufsize - 1] = '\0';
    return target;
}

/*
 * WARN: This is for external usage, don't modify it
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    size_t len = 0;
    struct list_head *li;

    list_for_each (li, head)
        ++len;

    return len;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
// https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
bool q_delete_mid(struct list_head *head)
{
#if 1
    /*
     * Use pointer of pointer to implement q_delete().
     */
    if (!head)
        return false;

    struct list_head **indir = &head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        indir = &(*indir)->next;
        fast = fast->next->next;
    }

    struct list_head *del = *indir;
    list_del_init(del);
    element_t *e = container_of(del->next, element_t, list);
    q_release_element(e);

    return true;

#else
    /*
     * Original implementation by using pointer.
     */
    if (!head)
        return false;

    struct list_head *fast = head->next, *slow = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    list_del_init(slow);
    element_t *e = container_of(slow->next, element_t, list);
    q_release_element(e);

    return true;
#endif
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
// https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return false;

    struct list_head *prev = head->next, *node = head->next->next;
    bool dup_flag = false;

    while (prev != head && node != head) {
        element_t *prev_element = list_entry(prev, element_t, list);
        element_t *node_element = list_entry(node, element_t, list);

        while (node != head &&
               strcmp(prev_element->value, node_element->value) == 0) {
            dup_flag = true;
            list_del(node);
            q_release_element(node_element);
            node = prev->next;
            node_element = list_entry(node, element_t, list);
        }

        if (dup_flag) {
            dup_flag = false;
            list_del(prev);
            q_release_element(prev_element);
        }

        prev = node;
        node = node->next;
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
// https://leetcode.com/problems/swap-nodes-in-pairs/
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *it = head->next;
    while (it != head && it->next != head) {
        struct list_head *tmp = it->next;
        list_del_init(it);
        list_add(it, tmp);
        it = it->next;
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *it, *tmp;
    for (it = head->next; it != head; it = it->prev) {
        tmp = it->next;
        it->next = it->prev;
        it->prev = tmp;
    }
    tmp = head->next;
    head->next = head->prev;
    head->prev = tmp;
}

/*
 * __q_merge() is an internal API for q_merge_sort().
 * Input: left and right are two sorted doubly linked lists.
 * Output: A sorted doubly linked list merged from left and right.
 */
struct list_head *__q_merge(struct list_head *left, struct list_head *right)
{
    struct list_head *head = NULL, **ptr = &head, **node;

    for (node = NULL; left && right; *node = (*node)->next) {
        if (strcmp(list_entry(left, element_t, list)->value,
                   list_entry(right, element_t, list)->value) < 0)
            node = &left;
        else
            node = &right;

        *ptr = *node;
        ptr = &(*ptr)->next;
    }
    *ptr = (struct list_head *) ((uintptr_t) left | (uintptr_t) right);

    return head;
}

struct list_head *q_merge_sort(struct list_head *head)
{
    if (!head->next)
        return head;

    struct list_head *slow = head, *fast = head;
    while (fast->next && fast->next->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    struct list_head *head2 = slow->next;
    slow->next = NULL;

    struct list_head *left = q_merge_sort(head), *right = q_merge_sort(head2);
    return __q_merge(left, right);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *node = head->next;
    head->prev->next = NULL;
    head->next = NULL;

    node = q_merge_sort(node);

    // We use doubly linked list to implement queue. However, q_merge_sort()
    // treat it as a singly linked list and only use the next pointer to sort
    // all the nodes. Use a loop and traverse along the next pointers to
    // rearrange the prev pointers.
    struct list_head *it = head;
    it->next = node;
    while (it->next) {
        it->next->prev = it;
        it = it->next;
    }
    it->next = head;
    head->prev = it;
}

bool q_shuffle(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return false;

    srand(time(NULL));

    int window = q_size(head);
    while (window) {
        int x = rand() % window;
        struct list_head *it = head->next;
        for (int i = 0; i < x; ++i)
            it = it->next;
        list_del_init(it);
        list_add(it, head);
        --window;
    }

    return true;
}
