/* FellaPC Firmaware
 * Universal two-way list
 * Copyright: Aleksander Kaminski, 2024
 * See LICENSE.md
 */

#ifndef LIST_H_
#define LIST_H_

/* One-way doubly linked list.
 * To use these functions:
 * two first fields has to be pointers to the
 * next and previous element:
 * struct example {
 *    struct example *next;
 *    struct example *prev;
 *    (..)
 * };
 */

void _list_push(void **list, void *elem);

void _list_append(void **list, void *elem);

void _list_pop(void **list, void *elem);

void _list_ufree(void **list);

#define list_push(l, e) _list_push((void **)(l), (void *)(e))
#define list_append(l, e) _list_append((void **)(l), (void *)(e))
#define list_pop(l, e) _list_pop((void **)(l), (void *)(e))
#define list_ufree(l) _list_ufree((void **)(l));

#endif
