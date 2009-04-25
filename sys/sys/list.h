/* Impala Operating System
 *
 * Copyright (C) 2009 University of Wroclaw. Department of Computer Science
 *    http://www.ii.uni.wroc.pl/
 * Copyright (C) 2009 Mateusz Kocielski, Artur Koninski, Pawel Wieczorek
 *    http://trzask.codepainters.com/impala/trac/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */
#ifndef __SYS_LIST_H
#define __SYS_LIST_H

// Proste listy, wzorowane na interfejsie z systemu Solaris

typedef struct list list_t;
typedef struct list_node list_node_t;
typedef bool list_less_f(const void *x, const void *y);
typedef bool list_pred_f(const void *x, void *arg);

struct list_node {
    void     *prev;
    void     *next;
};

struct list {
    uintptr_t   ls_offset;
    list_node_t ls_root;
    size_t      ls_length;
    bool        ls_cyclic;
};

#define __elem_node(ls, object)\
    ((list_node_t*)((ls)->ls_offset + (uintptr_t)(object)))

#define __next_node(ls, object)\
    __elem_node(ls,__elem_node(ls, object)->next)

#define __prev_node(ls, object)\
    __elem_node(ls,__elem_node(ls, object)->prev)


#define LIST_CREATE(ls, str,member,cyclic)\
    list_create(ls, offsetof(str,member),cyclic)

static inline void
list_create(list_t *ls, uintptr_t offset, bool cyclic)
{
    ls->ls_offset = offset;
    ls->ls_root.prev = NULL;
    ls->ls_root.next = NULL;
    ls->ls_length = 0;
    ls->ls_cyclic = cyclic;
}

static inline size_t
list_length(const list_t *ls)
{
    return ls->ls_length;
}

static inline bool
list_is_empty(const list_t *ls)
{
    return (ls->ls_length == 0);
}

static inline void* 
list_head(const list_t *ls)
{
    return ls->ls_root.next;
}

static inline void *
list_tail(const list_t *ls)
{
    return ls->ls_root.prev;
}

static inline void *
list_next(const list_t *ls, const void *elem)
{
    return (elem==NULL)? ls->ls_root.next : __elem_node(ls,elem)->next;
}

static inline void *
list_prev(const list_t *ls, const void *elem)
{
    return (elem==NULL)? ls->ls_root.prev : __elem_node(ls,elem)->prev;
}

static inline void *
list_remove(list_t *ls, void *x)
{
    list_node_t *x_node = __elem_node(ls, x);
    void *y = x_node->next;
    if (x_node->prev) __prev_node(ls, x)->next = x_node->next;
    if (x_node->next) __next_node(ls, x)->prev = x_node->prev;
    if (ls->ls_root.next == x) ls->ls_root.next = x_node->next;
    if (ls->ls_root.prev == x) ls->ls_root.prev = x_node->prev;
    x_node->next = NULL;
    x_node->prev = NULL;
    ls->ls_length--;
    return y;
}

static inline void *
list_extract_first(list_t *ls)
{
    void *x = list_head(ls);
    if (x) list_remove(ls, x);
    return x;
}

static inline void *
list_extract_last(list_t *ls)
{
    void *x = list_tail(ls);
    if (x) list_remove(ls, x);
    return x;
}


static inline void
__list_insert_after(list_t *ls, void *xs, void *x)
{
    list_node_t *xs_node = __elem_node(ls,xs);
    list_node_t *x_node = __elem_node(ls, x);
    x_node->prev = xs;
    x_node->next = xs_node->next;
    xs_node->next = x;
    if (x_node->next) __next_node(ls,x)->prev = x;
}

static inline void
__list_insert_before(list_t *ls, void *xs, void *x)
{
    list_node_t *xs_node = __elem_node(ls, xs);
    list_node_t *x_node = __elem_node(ls, x);
    x_node->next = xs;
    x_node->prev = xs_node->prev;
    xs_node->prev = x;
    if (x_node->prev) __prev_node(ls,x)->next = x;
}

static inline void 
list_insert_head(list_t *ls, void *elem)
{
    list_node_t *node = __elem_node(ls,elem);
    if (ls->ls_length == 0) {
        list_node_t *n = (ls->ls_cyclic)? elem : NULL;
        ls->ls_root.prev = elem;
        node->next = n;
        node->prev = n;
    } else {
        __list_insert_before(ls, ls->ls_root.next, elem);
    }
    ls->ls_root.next = elem;
    ls->ls_length++;
}


static inline void 
list_insert_tail(list_t *ls, void *elem)
{
    list_node_t *node = __elem_node(ls,elem);
    if (ls->ls_length == 0) {
        list_node_t *n = (ls->ls_cyclic)? elem : NULL;
        ls->ls_root.next = elem;
        node->next = n;
        node->prev = n;
    } else {
        __list_insert_after(ls, ls->ls_root.prev, elem);
    }
    ls->ls_root.prev = elem;
    ls->ls_length++;
}

static inline void
list_insert_after(list_t *ls, void *xs, void *x)
{
    if (xs == NULL) {
        list_insert_head(ls, x);
    } else
    if (xs == ls->ls_root.prev) {
        list_insert_tail(ls, x);
    } else {
        __list_insert_after(ls, xs, x);
    }
}

static inline void
list_insert_before(list_t *ls, void *xs, void *x)
{
    if (xs == NULL) {
        list_insert_tail(ls, x);
    } else
    if (xs == ls->ls_root.next) {
        list_insert_head(ls, x);
    } else {
        __list_insert_before(ls, xs, x);
    }
}

// Nie u�ywa� przy listach cyklicznych!
static inline void
list_insert_in_order(list_t *ls, void *x, list_less_f *is_less)
{
    void *elem = NULL;
    // lecimy po liscie dopoki: elem < x
    while ( (elem = list_next(ls, elem)) && is_less(elem,x) );
    if (elem == NULL) {
        list_insert_tail(ls, x);
    } else {
        list_insert_before(ls, elem, x);
    }
}

static inline void *
list_find_next(list_t *ls, void *elem, list_pred_f *pred, void *parg)
{
    while ( (elem = list_next(ls, elem)) ) {
        if (pred(elem, parg)) return elem;
    }
    return NULL;
}

static inline void *
list_find(list_t *ls, list_pred_f *pred, void *parg)
{
    return list_find_next(ls, NULL, pred, parg);
}


#undef __elem_node
#undef __next_node
#undef __prev_node


#endif

