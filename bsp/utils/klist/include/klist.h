/**
 * @file klist.h
 * @brief A doubly linked list implementation.
 * @author Peter.Peng <27144363@qq.com>
 * @date 2022
 *
 * Embedded Device Software
 * Copyright (C) 2022 Peter.Peng
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __KLIST_H__
#define __KLIST_H__

/**
 * @defgroup list List
 *
 * @brief A doubly linked list implementation.
 *
 * @ingroup utils_group
 *
 * @{
 *
 */

/**
 * @brief   Cast a member of a structure out to the containing structure.
 *
 * @param   ptr The pointer to the member.
 * @param   type The type of the container struct this is embedded in.
 * @param   member The name of the member within the struct.
 */
#define container_of(ptr, type, member) ({ \
        void* __mptr = (void*)(ptr); \
        ((type*)(__mptr - offsetof(type, member))); })

/**
 * @brief   Define list_head structure.
 */
struct list_head
{
    struct list_head*   next;   /**< Pointer to the next node. */
    struct list_head*   prev;   /**< Pointer to the previous node. */
};

/**
 * @brief   Initialize a list_head structure.
 */
#define LIST_HEAD_INIT(name) { &(name), &(name) }

/**
 * @brief   Define and initialize a list_head structure.
 */
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

/**
 * @brief   Initialize a list_head structure.
 *
 * @param   list list_head structure to be initialized.
 */
static inline void INIT_LIST_HEAD(struct list_head* list)
{
    list->next = list;
    list->prev = list;
}

#ifndef DOC_HIDDEN
static inline void __list_add(struct list_head* new,
                              struct list_head* prev,
                              struct list_head* next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}
#endif

/**
 * @brief   Add a new entry.
 *
 * @param   new New entry to be added.
 * @param   head List head to add it after.
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(struct list_head* new, struct list_head* head)
{
    __list_add(new, head, head->next);
}

/**
 * @brief   Add a new entry at the tail of the list.
 *
 * @param   new New entry to be added.
 * @param   head List head to add it before.
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head* new, struct list_head* head)
{
    __list_add(new, head->prev, head);
}

#ifndef DOC_HIDDEN
static inline void __list_del(struct list_head* prev, struct list_head* next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void __list_del_entry(struct list_head* entry)
{
    __list_del(entry->prev, entry->next);
}
#endif

/**
 * @brief   Deletes entry from list.
 *
 * @param   entry The element to delete from the list.
 *
 * @note    list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void list_del(struct list_head* entry)
{
    __list_del_entry(entry);
    entry->next = NULL;
    entry->prev = NULL;
}

/**
 * @brief   Replace old entry by new one.
 *
 * @param   old The element to be replaced.
 * @param   new The new element to insert.
 *
 * @note    If old was empty, it will be overwritten.
 */
static inline void list_replace(struct list_head* old, struct list_head* new)
{
    new->next = old->next;
    new->next->prev = new;
    new->prev = old->prev;
    new->prev->next = new;
}

/**
 * @brief   Replace old entry by new one and initialize the old one.
 *
 * @param   old The element to be replaced.
 * @param   new The new element to insert.
 *
 * @note    If old was empty, it will be overwritten.
 */
static inline void list_replace_init(struct list_head* old,
                                     struct list_head* new)
{
    list_replace(old, new);
    INIT_LIST_HEAD(old);
}

/**
 * @brief   Replace entry1 with entry2 and re-add entry1 at entry2's position.
 *
 * @param   entry1 The location to place entry2.
 * @param   entry2 The location to place entry1.
 */
static inline void list_swap(struct list_head* entry1, struct list_head* entry2)
{
    struct list_head* pos = entry2->prev;

    list_del(entry2);
    list_replace(entry1, entry2);
    if (pos == entry1)
    {
        pos = entry2;
    }
    list_add(entry1, pos);
}

/**
 * @brief   Deletes entry from list and reinitialize it.
 *
 * @param   entry The element to delete from the list.
 */
static inline void list_del_init(struct list_head* entry)
{
    __list_del_entry(entry);
    INIT_LIST_HEAD(entry);
}

/**
 * @brief   Delete from one list and add as another's head.
 *
 * @param   list The entry to move.
 * @param   head The head that will precede our entry.
 */
static inline void list_move(struct list_head* list, struct list_head* head)
{
    __list_del_entry(list);
    list_add(list, head);
}

/**
 * @brief   Delete from one list and add as another's tail.
 *
 * @param   list The entry to move.
 * @param   head The head that will follow our entry.
 */
static inline void list_move_tail(struct list_head* list,
                                  struct list_head* head)
{
    __list_del_entry(list);
    list_add_tail(list, head);
}

/**
 * @brief   Move a subsection of a list to its tail.
 *
 * @param   head The head that will follow our entry.
 * @param   first First entry to move.
 * @param   last Last entry to move, can be the same as first.
 *
 * Move all entries between first and including last before head.
 * All three entries must belong to the same linked list.
 */
static inline void list_bulk_move_tail(struct list_head*    head,
                                       struct list_head*    first,
                                       struct list_head*    last)
{
    first->prev->next = last->next;
    last->next->prev = first->prev;

    head->prev->next = first;
    first->prev = head->prev;

    last->next = head;
    head->prev = last;
}

/**
 * @brief   Tests whether list is the first entry in list head.
 *
 * @param   list The entry to test.
 * @param   head The head of the list.
 *
 * @retval  True if list is first, otherwise a false.
 */
static inline int list_is_first(const struct list_head* list,
                                const struct list_head* head)
{
    return list->prev == head;
}

/**
 * @brief   Tests whether list is the last entry in list head.
 *
 * @param   list The entry to test.
 * @param   head The head of the list.
 *
 * @retval  True if list is last, otherwise a false.
 */
static inline int list_is_last(const struct list_head*  list,
                               const struct list_head*  head)
{
    return list->next == head;
}

/**
 * @brief   Tests whether list is the list head.
 *
 * @param   list The entry to test.
 * @param   head The head of the list.
 *
 * @retval  True if list is a head, otherwise a false.
 */
static inline int list_is_head(const struct list_head*  list,
                               const struct list_head*  head)
{
    return list == head;
}

/**
 * @brief   Tests whether a list is empty.
 *
 * @param   head The list to test.
 *
 * @retval  True if head is empty, otherwise a false.
 */
static inline int list_empty(const struct list_head* head)
{
    return head->next == head;
}

/**
 * @brief   Rotate the list to the left.
 *
 * @param   head The head of the list.
 */
static inline void list_rotate_left(struct list_head* head)
{
    struct list_head* first;

    if (!list_empty(head))
    {
        first = head->next;
        list_move_tail(first, head);
    }
}

/**
 * @brief   Rotate list to specific item.
 *
 * @param   list The desired new front of the list.
 * @param   head The head of the list.
 *
 * Rotates list so that list becomes the new front of the list.
 */
static inline void list_rotate_to_front(struct list_head*   list,
                                        struct list_head*   head)
{
    /*
     * Deletes the list head from the list denoted by @head and
     * places it as the tail of @list, this effectively rotates the
     * list so that @list is at the front.
     */
    list_move_tail(head, list);
}

/**
 * @brief   Tests whether a list has just one entry.
 *
 * @param   head The list to test.
 *
 * @retval  True if head is singular, otherwise a false.
 */
static inline int list_is_singular(const struct list_head* head)
{
    return !list_empty(head) && (head->next == head->prev);
}

#ifndef DOC_HIDDEN
static inline void __list_cut_position(struct list_head*    list,
                                       struct list_head*    head,
                                       struct list_head*    entry)
{
    struct list_head* new_first = entry->next;

    list->next = head->next;
    list->next->prev = list;
    list->prev = entry;
    entry->next = list;
    head->next = new_first;
    new_first->prev = head;
}
#endif

/**
 * @brief   Cut a list into two.
 *
 * @param   list A new list to add all removed entries.
 * @param   head A list with entries.
 * @param   entry An entry within head, could be the head itself
 *          and if so we won't cut the list.
 *
 * This helper moves the initial part of head, up to and
 * including entry, from head to list. You should
 * pass on entry an element you know is on head. list
 * should be an empty list or a list you do not care about
 * losing its data.
 */
static inline void list_cut_position(struct list_head*  list,
                                     struct list_head*  head,
                                     struct list_head*  entry)
{
    if (list_empty(head))
    {
        return;
    }
    if (list_is_singular(head) &&
        !list_is_head(entry, head) && (entry != head->next))
    {
        return;
    }
    if (list_is_head(entry, head))
    {
        INIT_LIST_HEAD(list);
    }
    else
    {
        __list_cut_position(list, head, entry);
    }
}

/**
 * @brief   Cut a list into two, before given entry.
 *
 * @param   list A new list to add all removed entries.
 * @param   head A list with entries.
 * @param   entry An entry within head, could be the head itself.
 *
 * This helper moves the initial part of head, up to but
 * excluding entry, from head to list. You should pass
 * in entry an element you know is on head. list should
 * be an empty list or a list you do not care about losing
 * its data.
 * If entry == head, all entries on head are moved to list.
 */
static inline void list_cut_before(struct list_head*    list,
                                   struct list_head*    head,
                                   struct list_head*    entry)
{
    if (head->next == entry)
    {
        INIT_LIST_HEAD(list);
        return;
    }
    list->next = head->next;
    list->next->prev = list;
    list->prev = entry->prev;
    list->prev->next = list;
    head->next = entry;
    entry->prev = head;
}

#ifndef DOC_HIDDEN
static inline void __list_splice(const struct list_head*    list,
                                 struct list_head*          prev,
                                 struct list_head*          next)
{
    struct list_head* first = list->next;
    struct list_head* last = list->prev;

    first->prev = prev;
    prev->next = first;

    last->next = next;
    next->prev = last;
}
#endif

/**
 * @brief   Join two lists, this is designed for stacks.
 *
 * @param   list The new list to add.
 * @param   head The place to add it in the first list.
 */
static inline void list_splice(const struct list_head*  list,
                               struct list_head*        head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head, head->next);
    }
}

/**
 * @brief   Join two lists, each list being a queue.
 *
 * @param   list The new list to add.
 * @param   head The place to add it in the first list.
 */
static inline void list_splice_tail(struct list_head*   list,
                                    struct list_head*   head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head->prev, head);
    }
}

/**
 * @brief   Join two lists and reinitialise the emptied list.
 *
 * @param   list The new list to add.
 * @param   head The place to add it in the first list.
 *
 * The list at list is reinitialised
 */
static inline void list_splice_init(struct list_head*   list,
                                    struct list_head*   head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head, head->next);
        INIT_LIST_HEAD(list);
    }
}

/**
 * @brief   Join two lists and reinitialise the emptied list.
 *
 * @param   list The new list to add.
 * @param   head The place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at list is reinitialised.
 */
static inline void list_splice_tail_init(struct list_head*  list,
                                         struct list_head*  head)
{
    if (!list_empty(list))
    {
        __list_splice(list, head->prev, head);
        INIT_LIST_HEAD(list);
    }
}

/**
 * @brief   Get the struct for this entry.
 *
 * @param   ptr The struct list_head pointer.
 * @param   type The type of the struct this is embedded in.
 * @param   member The name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/**
 * @brief   Get the first element from a list.
 *
 * @param   ptr The list head to take the element from.
 * @param   type The type of the struct this is embedded in.
 * @param   member The name of the list_head within the struct.
 *
 * @note    That list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
    list_entry((ptr)->next, type, member)

/**
 * @brief   Get the last element from a list.
 *
 * @param   ptr The list head to take the element from.
 * @param   type The type of the struct this is embedded in.
 * @param   member The name of the list_head within the struct.
 *
 * @note    That list is expected to be not empty.
 */
#define list_last_entry(ptr, type, member) \
    list_entry((ptr)->prev, type, member)

/**
 * @brief   Get the first element from a list.
 *
 * @param   ptr The list head to take the element from.
 * @param   type The type of the struct this is embedded in.
 * @param   member The name of the list_head within the struct.
 *
 * @note    That if the list is empty, it returns NULL.
 */
#define list_first_entry_or_null(ptr, type, member) ({ \
        struct list_head* head__ = (ptr); \
        struct list_head* pos__ = head__->next; \
        pos__ != head__ ? list_entry(pos__, type, member) : NULL; \
    })

/**
 * @brief   Get the next element in list.
 *
 * @param   pos The type * to cursor.
 * @param   member The name of the list_head within the struct.
 */
#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

/**
 * @brief   Get the next element in list.
 *
 * @param   pos The type * to cursor.
 * @param   head The list head to take the element from.
 * @param   member The name of the list_head within the struct.
 *
 * Wraparound if pos is the last element (return the first element).
 *
 * @note    That list is expected to be not empty.
 */
#define list_next_entry_circular(pos, head, member) \
    (list_is_last(&(pos)->member, head) ? \
     list_first_entry(head, typeof(*(pos)), member) : list_next_entry(pos, \
                                                                      member))

/**
 * @brief   Get the prev element in list.
 *
 * @param   pos The type * to cursor.
 * @param   member The name of the list_head within the struct.
 */
#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)

/**
 * @brief   Get the prev element in list.
 *
 * @param   pos The type * to cursor.
 * @param   head The list head to take the element from.
 * @param   member The name of the list_head within the struct.
 *
 * Wraparound if pos is the first element (return the last element).
 *
 * @note    That list is expected to be not empty.
 */
#define list_prev_entry_circular(pos, \
                                 head, \
                                 member) \
    (list_is_first(&(pos)->member, head) ? \
     list_last_entry(head, typeof(*(pos)), member) : list_prev_entry(pos, \
                                                                     member))

/**
 * @brief   Iterate over a list.
 *
 * @param   pos The struct list_head to use as a loop cursor.
 * @param   head The head for your list.
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; !list_is_head(pos, (head)); pos = pos->next)

/**
 * @brief   Iterate backwards over a list.
 *
 * @param   pos The struct list_head to use as a loop cursor.
 * @param   head The head for your list.
 */
#define list_for_each_reverse(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * @brief   Continue iteration over a list.
 *
 * @param   pos The struct list_head to use as a loop cursor.
 * @param   head The head for your list.
 *
 * Continue to iterate over a list, continuing after the current position.
 */
#define list_for_each_continue(pos, head) \
    for (pos = pos->next; !list_is_head(pos, (head)); pos = pos->next)

/**
 * @brief   Iterate over a list backwards.
 *
 * @param   pos The struct list_head to use as a loop cursor.
 * @param   head The head for your list.
 */
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; !list_is_head(pos, (head)); pos = pos->prev)

/**
 * @brief   Iterate over a list safe against removal of list entry.
 *
 * @param   pos The struct list_head to use as a loop cursor.
 * @param   n Another struct list_head to use as temporary storage.
 * @param   head The head for your list.
 */
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; \
         !list_is_head(pos, (head)); \
         pos = n, n = pos->next)

/**
 * @brief   Iterate over a list backwards safe against removal of list entry.
 *
 * @param   pos The struct list_head to use as a loop cursor.
 * @param   n Another struct list_head to use as temporary storage.
 * @param   head The head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) \
    for (pos = (head)->prev, n = pos->prev; \
         !list_is_head(pos, (head)); \
         pos = n, n = pos->prev)

/**
 * @brief   Count nodes in the list.
 *
 * @param   head The head for your list.
 *
 * @retval  Returns the count of nodes.
 */
static inline size_t list_count_nodes(struct list_head* head)
{
    struct list_head* pos;
    size_t count = 0;

    list_for_each(pos, head)
    count++;

    return count;
}

/**
 * @brief   Test if the entry points to the head of the list.
 *
 * @param   pos The type * to cursor.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 */
#define list_entry_is_head(pos, head, member) \
    list_is_head(&pos->member, (head))

/**
 * @brief   Iterate over list of given type.
 *
 * @param   pos The type * to use as a loop cursor.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 */
#define list_for_each_entry(pos, head, member)              \
    for (pos = list_first_entry(head, typeof(*pos), member); \
         !list_entry_is_head(pos, head, member);            \
         pos = list_next_entry(pos, member))

/**
 * @brief   Iterate backwards over list of given type.
 *
 * @param   pos The type * to use as a loop cursor.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)          \
    for (pos = list_last_entry(head, typeof(*pos), member);     \
         !list_entry_is_head(pos, head, member);            \
         pos = list_prev_entry(pos, member))

/**
 * @brief   Prepare a pos entry for use in list_for_each_entry_continue().
 *
 * @param   pos The type * to use as a start point.
 * @param   head The head of the list.
 * @param   member The name of the list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
    ((pos) ? : list_entry(head, typeof(*pos), member))

/**
 * @brief   Continue iteration over list of given type.
 *
 * @param   pos The type * to use as a loop cursor.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member) \
    for (pos = list_next_entry(pos, member); \
         !list_entry_is_head(pos, head, member); \
         pos = list_next_entry(pos, member))

/**
 * @brief   Iterate backwards from the given point.
 *
 * @param   pos The type * to use as a loop cursor.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, member) \
    for (pos = list_prev_entry(pos, member); \
         !list_entry_is_head(pos, head, member); \
         pos = list_prev_entry(pos, member))

/**
 * @brief   Iterate over list of given type from the current point.
 * @param   pos The type * to use as a loop cursor.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, member) \
    for (; !list_entry_is_head(pos, head, member); \
         pos = list_next_entry(pos, member))

/**
 * @brief   Iterate backwards over list of given type from the current point.
 *
 * @param   pos The type * to use as a loop cursor.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, continuing from current position.
 */
#define list_for_each_entry_from_reverse(pos, head, member)     \
    for (; !list_entry_is_head(pos, head, member);          \
         pos = list_prev_entry(pos, member))

/**
 * @brief   Iterate over list of given type safe against removal of list entry.
 *
 * @param   pos The type * to use as a loop cursor.
 * @param   n Another type * to use as temporary storage
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member), \
         n = list_next_entry(pos, member); \
         !list_entry_is_head(pos, head, member); \
         pos = n, n = list_next_entry(n, member))

/**
 * @brief   Continue list iteration safe against removal.
 *
 * @param   pos The type * to use as a loop cursor.
 * @param   n Another type * to use as temporary storage.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, head, member) \
    for (pos = list_next_entry(pos, member), \
         n = list_next_entry(pos, member); \
         !list_entry_is_head(pos, head, member); \
         pos = n, n = list_next_entry(n, member))

/**
 * @brief   Iterate over list from current point safe against removal.
 *
 * @param   pos The type * to use as a loop cursor.
 * @param   n Another type * to use as temporary storage.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, head, member) \
    for (n = list_next_entry(pos, member); \
         !list_entry_is_head(pos, head, member); \
         pos = n, n = list_next_entry(n, member))

/**
 * @brief   Iterate backwards over list safe against removal.
 *
 * @param   pos The type * to use as a loop cursor.
 * @param   n Another type * to use as temporary storage.
 * @param   head The head for your list.
 * @param   member The name of the list_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member) \
    for (pos = list_last_entry(head, typeof(*pos), member), \
         n = list_prev_entry(pos, member); \
         !list_entry_is_head(pos, head, member); \
         pos = n, n = list_prev_entry(n, member))

/**
 * @brief   Reset a stale list_for_each_entry_safe loop.
 *
 * @param   pos The loop cursor used in the list_for_each_entry_safe loop.
 * @param   n Temporary storage used in list_for_each_entry_safe.
 * @param   member The name of the list_head within the struct.
 *
 * list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define list_safe_reset_next(pos, n, member) \
    n = list_next_entry(pos, member)

/**
 * @}
 */

#endif /* __KLIST_H__ */
