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


#ifndef PQUEUE_H_
#define PQUEUE_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include "pqueue.h"

#define MPANIC(x)	; assert(x != NULL)
#define PQLOCK(heap)	int mutex_status = pthread_mutex_lock(heap); if (mutex_status != 0) goto error
#define PQUNLOCK(heap)	pthread_mutex_unlock(heap)

static int insert_node(Priqueue *heap, Node *node);
static Node *pop_node(Priqueue *heap);
static Priqueue *popall(Priqueue *heap);


MHEAP_API Priqueue *priqueue_initialize(unsigned int initial_length, unsigned int blocking, unsigned int limit_length) {
	int mutex_status, cond_status;

	initial_length++;

	Priqueue *heap = malloc(sizeof(*heap)) MPANIC(heap);
	const size_t hsize = initial_length * sizeof(*heap->array);

	mutex_status = pthread_mutex_init(&(heap->lock), NULL);
	if (mutex_status != 0) goto error;

	if (blocking) {
		cond_status = pthread_cond_init(&(heap->not_empty), NULL);
		if (cond_status != 0) goto error;
	}

	heap->head = NULL;
	heap->heap_size = initial_length;
	heap->limit_size = limit_length;
	heap->current = 0;
	heap->next_id = 1;
	heap->blocking = blocking;
	heap->array = malloc(hsize) MPANIC(heap->array);

	memset(heap->array, 0x00, hsize);

	return heap;

error:
	free(heap);
	return NULL;
}

static MHEAP_API MHEAPSTATUS realloc_heap(Priqueue *heap) {
	if (heap->current == heap->heap_size) {
		const size_t arrsize = sizeof(*heap->array);

		Node **resized_heap = NULL;
		resized_heap = realloc(heap->array, ((2 * heap->heap_size) + 1) * arrsize);

		if (resized_heap != NULL) {
			heap->heap_size *= 2;
			heap->array = (Node**) resized_heap;

			int i = heap->current + 1;
			for(; i < heap->heap_size; i++)
				*(heap->array + i) = NULL;

			return MHEAP_OK;
		} else{
			return MHEAP_REALLOCERROR;
		}
	}

	return MHEAP_NOREALLOC;
}

MHEAP_API int priqueue_insert(Priqueue *heap, Data *data, uintptr_t priority) {
	Node *node = malloc(sizeof(*node)) MPANIC(node);
	int ret = -1;

	node->priority = priority;
	node->data = data;

	PQLOCK(&(heap->lock));
	ret = insert_node(heap, node);
	PQUNLOCK(&(heap->lock));
	if (heap->blocking)
		pthread_cond_signal(&(heap->not_empty));

	return ret;

error:
	node->data = NULL;
	free(node);
	return -1;
}

static int insert_node(Priqueue *heap, Node *node) {
	if (heap->current == heap->heap_size) {
		if (heap->limit_size > 0) {
			priqueue_node_free(heap, node);
			return -1;
		} else {
			unsigned int realloc_status = realloc_heap(heap);
			assert(realloc_status == MHEAP_OK);
		}
	}

	if (heap->next_id == ULLONG_MAX) {
		Priqueue *tmp_heap = popall(heap);

		heap->next_id = 1;
		Node *item = NULL;

		while ((item = pop_node(tmp_heap)) != NULL)
			insert_node(heap, item);

		priqueue_free(tmp_heap);
	}

	node->id = heap->next_id;
	heap->next_id++;

	heap->current++;

	int hole = heap->current;

	if (heap->array[hole / 2] != NULL) {
		for (; hole > 1 && node->priority < heap->array[hole / 2]->priority; hole /= 2)
			heap->array[hole] = heap->array[hole / 2];
	}

	heap->array[hole] = node;

	return 0;
}

MHEAP_API Node *priqueue_pop(Priqueue *heap) {
	Node *node = NULL;

	PQLOCK(&(heap->lock));
	if (heap->blocking)
		while(heap->current == 0)
			pthread_cond_wait(&(heap->not_empty), &(heap->lock));
	node = pop_node(heap);
	PQUNLOCK(&(heap->lock));

	return node;

error:
	return NULL;
}

static void percolate_down(Priqueue *heap, int hole) {
	Node *tmp = heap->array[hole];
	int child;

	for(; hole * 2 <= heap->current; hole = child) {
		child = hole * 2;

		if ((child != heap->current) && 
			((heap->array[child+1]->priority < heap->array[child]->priority) ||
			((heap->array[child+1]->priority == heap->array[child]->priority) && (heap->array[child + 1]->id < heap->array[child]->id)))) {
			child++;
		}

		if ((heap->array[child]->priority < tmp->priority) ||
			((heap->array[child]->priority == tmp->priority) && (heap->array[child]->id < tmp->id))) {
			heap->array[hole] = heap->array[child];
		} else {
			break;
		}
	}

	heap->array[hole] = tmp;
}

static Node *pop_node(Priqueue *heap) {
	Node *node = NULL;

	if (heap->current > 0) {
		node = heap->array[1];
		heap->array[1] = heap->array[heap->current];

		percolate_down(heap, 1);

		heap->current -= 1;
	}

	return node;
}

MHEAP_API void priqueue_free(Priqueue *heap) {
	if (heap->current > 0) {
		unsigned int i = 1;
		for (; i <= heap->current; i++)
			priqueue_node_free(heap, heap->array[i]);
	}

	free(*heap->array);
	free(heap->array);
	free(heap);
}

MHEAP_API void priqueue_node_free(Priqueue *heap, Node *node) {
	if (node != NULL) {
		if (node->data->data != NULL)
			free(node->data->data);
		free(node->data);
		free(node);
	}
}

static Priqueue *popall(Priqueue *heap) {
	Priqueue *result = priqueue_initialize(heap->heap_size, heap->blocking, heap->limit_size);

	Node *item = NULL;

	while ((item = pop_node(heap)) != NULL)
		insert_node(result, item);

	return result;
}

MHEAP_API Priqueue *priqueue_popall(Priqueue *heap) {
	Priqueue *new_heap = NULL;

	if (heap == NULL) goto error;

	PQLOCK(&(heap->lock));
	new_heap = popall(heap);
	PQUNLOCK(&(heap->lock));

	return new_heap;

error:
	return NULL;
}

MHEAP_API int priqueue_isempty(Priqueue *heap) {
	if (heap == NULL) goto error;

	if (heap->current == 0) // empty
		return 0;
	else // not empty
		return -1;

error:
	return -1;
}

MHEAP_API Node *priqueue_peek(Priqueue *heap) {
	Node *node = NULL;

	if (heap == NULL) goto error;

	if (priqueue_isempty(heap) < 0)
		node = heap->array[1];

	return node;

error:
	return NULL;
}

#endif
