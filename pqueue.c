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
#include "pqueue.h"

#define GAP 2

#define MPANIC(x)	; assert(x != NULL)
#define PQLOCK(heap)	int mutex_status = pthread_mutex_lock(heap); if (mutex_status != 0) goto error
#define PQUNLOCK(heap)	pthread_mutex_unlock(heap)

static void insert_node(Priqueue *heap, Node *node);
static Node *pop_node(Priqueue *heap);
static void swap_node(Priqueue *heap, unsigned int a, unsigned int b);
static Priqueue *popall(Priqueue *heap);


MHEAP_API Priqueue *priqueue_initialize(unsigned int initial_length, unsigned int blocking) {
	int mutex_status, cond_status;

	Priqueue *heap  = malloc(sizeof(*heap)) MPANIC(heap);
	const size_t hsize = initial_length * sizeof(*heap->array);

	mutex_status = pthread_mutex_init(&(heap->lock), NULL);
	if (mutex_status != 0) goto error;

	if (blocking) {
		cond_status = pthread_cond_init(&(heap->not_empty), NULL);
		if (cond_status != 0) goto error;
	}

	heap->head = NULL;
	heap->heap_size = initial_length;
	heap->occupied = 1;
	heap->current = 1;
	heap->blocking = blocking;
	heap->array = malloc(hsize) MPANIC(heap->array);

	memset(heap->array, 0x00, hsize);

	return heap;

error:
	free(heap);
	return NULL;
}

static MHEAP_API MHEAPSTATUS realloc_heap(Priqueue *heap) {
	if (heap->occupied == heap->heap_size) {
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

MHEAP_API void priqueue_insert(Priqueue *heap, Data *data, uintptr_t priority) {
	Node *node = malloc(sizeof(*node)) MPANIC(node);
	node->priority = priority;
	node->data = data;

	PQLOCK(&(heap->lock));
	insert_node(heap, node);
	PQUNLOCK(&(heap->lock));
	if (heap->blocking)
		pthread_cond_signal(&(heap->not_empty));

error:
	node->data = NULL;
	free(node);
}

static void insert_node(Priqueue *heap, Node* node) {
	if ((heap->current == 1) && (heap->array[1] == NULL)) {
		heap->head = node;
		heap->array[1] = node;
		heap->array[1]->index = heap->current;
		heap->occupied++;
		heap->current++;

		return;
	}

	if (heap->occupied == heap->heap_size) {
		unsigned int realloc_status = realloc_heap(heap);
		assert(realloc_status == MHEAP_OK);
	}

	if(heap->occupied <= heap->heap_size) {
		node->index = heap->current;
		heap->array[heap->current] = node;

		int parent = heap->current / GAP;

		if ((heap->array[parent]) && (heap->array[parent]->priority < node->priority)) {
			heap->occupied++;
			heap->current++;

			int depth = heap->current / GAP;
			int traverse = node->index;

			while (depth >= 1) {
				if (traverse == 1) break;

				unsigned int parent = traverse / GAP;

				if (heap->array[parent]->priority < heap->array[traverse]->priority) {
					swap_node(heap, parent, traverse);
					traverse = heap->array[parent]->index;
				}

				depth--;
			}

			heap->head = heap->array[1];
		} else {
			heap->occupied++;
			heap->current++;
		}
	}

	return;
}

void swap_node(Priqueue *heap, unsigned int parent, unsigned int child) {
	Node *tmp = heap->array[parent];

	heap->array[parent] = heap->array[child];
	heap->array[parent]->index = tmp->index;

	heap->array[child] = tmp;
	heap->array[child]->index = child;
}

MHEAP_API Node *priqueue_pop(Priqueue *heap) {
	Node *node = NULL;

	PQLOCK(&(heap->lock));
	if (heap->blocking)
		while(heap->current == 1)
			pthread_cond_wait(&(heap->not_empty), &(heap->lock));
	node = pop_node(heap);
	PQUNLOCK(&(heap->lock));

	return node;

error:
	return NULL;
}

static Node *pop_node(Priqueue *heap) {
	Node *node = NULL;
	unsigned int i, depth;

	if (heap->current == 1) {
		return node;
	} else if (heap->current >= 2) {
		node = heap->array[1];
		heap->array[1] = heap->array[heap->current - 1];
		heap->array[heap->current - 1] = NULL;

		if (heap->array[1] != NULL) heap->array[1]->index = 1;

		heap->current -= 1;
		heap->occupied -= 1;

		depth = (heap->current - 1) / 2;

		for(i = 1; i<=depth; i++) {
			if (!(heap->array[i]) && ((heap->array[i * GAP]) && (heap->array[(i * GAP)+1]))) continue;

			if ((heap->array[i]->priority < heap->array[i * GAP]->priority) ||
				(heap->array[i]->priority < heap->array[(i * GAP) + 1]->priority)) {
				unsigned int max = (heap->array[i * GAP]->priority > heap->array[(i * GAP) + 1]->priority) ?
					heap->array[(i * GAP)]->index : heap->array[(i * GAP) + 1]->index;

				swap_node(heap, i, max);
			}
		}
	}

	return node;
}

MHEAP_API void priqueue_free(Priqueue *heap) {
	if (heap->current >= 2) {
		unsigned int i = 1;
		for (; i < heap->current; i++)
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
	Priqueue *result = priqueue_initialize(heap->heap_size, heap->blocking);

	Node* item = priqueue_pop(heap);

	while (item != NULL) {
		item = priqueue_pop(heap);

		if(item != NULL)
			insert_node(result, item);
	}

	return result;
}

MHEAP_API Priqueue *priqueue_popall(Priqueue *heap) {
	Priqueue *new_heap = NULL;

	PQLOCK(&(heap->lock));
	new_heap = popall(heap);
	PQUNLOCK(&(heap->lock));

	return new_heap;

error:
	return NULL;
}

#endif
