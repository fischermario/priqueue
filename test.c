/*
The MIT License (MIT)

Copyright (c) 2015 Mike Taghavi (mitghi) <mitghi@me.com>

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

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "pqueue.h"

#define MAX_ITEMS 10

#define CHECK_COND(cond) if (__sync_bool_compare_and_swap(&cond, 1, 1)) break;
#define SWAP_COND(cond, a, b) while(1) { if (__sync_bool_compare_and_swap(&cond, a, b)) break; }

volatile unsigned int cond = 0;

enum {
	TYPE_CHAR,
	TYPE_INT,
	TYPE_FLOAT
};

void *producer(void *arg) {
	Priqueue *h = (Priqueue *)arg;

	printf("Producer %u started!\n", (unsigned int)pthread_self());

	unsigned int i = 1;
	for(; i < MAX_ITEMS; i++) {
		Data *value = (Data *) malloc(sizeof(Data));
		value->type = TYPE_CHAR;
		value->data = (char *) malloc(7 * sizeof(char *));
		sprintf(value->data, "test %d", i);

		printf("Insert '%s' with priority %d\n", (char *) value->data, MAX_ITEMS-i);

		priqueue_insert(h, value, MAX_ITEMS - i);
	}

	printf("Producer %u finished!\n", (unsigned int)pthread_self());

	SWAP_COND(cond, 0, 1);

	return NULL;
}

void *consumer(void *arg) {
	Node *d = NULL;
	Priqueue *h = (Priqueue *)arg;

	printf("Consumer %u started!\n", (unsigned int)pthread_self());

	for (;;) {
		sched_yield();
		CHECK_COND(cond);
	}

	printf("Consumer %u ready to process...\n", (unsigned int)pthread_self());

	for (;;) {
		d = priqueue_pop(h);

		if (d != NULL) {
			printf("Consumer %u: Remove '%s' with priority %lu\n", (unsigned int)pthread_self(), (char *)d->data->data, d->priority);

			priqueue_node_free(h, d);
		} else {
			break;
		}
	}

	return NULL;
}

int main() {
	pthread_t t1, t2, t3;

	Priqueue *heap = priqueue_initialize(MAX_ITEMS, 0, 0);

	if (heap != NULL) {
		printf("Priqueue successfully initialized!\n");
	} else {
		printf("Error initializing Priqueue! Exit...\n");
		return 1;
	}

	pthread_create(&t1, NULL, producer, (void *)heap);
	pthread_create(&t2, NULL, consumer, (void *)heap);
	pthread_create(&t3, NULL, consumer, (void *)heap);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);

	printf("Finished! Cleaning up...\n");
	
	priqueue_free(heap);

	return 0;
}
