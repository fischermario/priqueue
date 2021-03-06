/*
The MIT License (MIT)

Copyright (c) 2016 Mike Taghavi (mitghi) <mitghi@me.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/



#ifndef MHEAP_H
#define MHEAP_H

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>

#define MHEAP_API

typedef struct _data Data;
typedef struct _node Node;
typedef struct _heap Priqueue;
typedef struct _iterator Priqueue_Iterator;

struct _data {
	unsigned int type;
	void *data;
};

struct _node {
	uintptr_t priority;
	unsigned int index;
	unsigned long long id;
	struct _data *data;
	struct _node *next;
};

struct _heap {
	struct _node *head;
	struct _node **array;
	unsigned int heap_size;
	unsigned int limit_size;
	unsigned int current;
	unsigned int blocking;
	unsigned long long next_id;
	pthread_mutex_t lock;
	pthread_cond_t not_empty;
};

struct _iterator {
	struct _heap *heap;
	unsigned int index;
	uintptr_t priority;
};

typedef enum {
	MHEAP_OK = 0,
	MHEAP_EMPTY,
	MHEAP_FAILED,
	MHEAP_REALLOCERROR,
	MHEAP_NOREALLOC,
	MHEAP_FATAL
} MHEAPSTATUS;


Priqueue *
priqueue_initialize(unsigned int, unsigned int, unsigned int);

int
priqueue_insert(Priqueue *, Data *, uintptr_t);

Node *
priqueue_pop(Priqueue *);

void
priqueue_free(Priqueue *);

void
priqueue_node_free(Priqueue *, Node *);

Priqueue *
priqueue_popall(Priqueue *);

int
priqueue_isempty(Priqueue *);

Node *
priqueue_peek(Priqueue *);

int
priqueue_getsize(Priqueue *);

Priqueue_Iterator *
priqueue_iterator_create(Priqueue *);

Node *
priqueue_iterator_get_next(Priqueue_Iterator *);

int
priqueue_iterator_free(Priqueue_Iterator *);

#endif
