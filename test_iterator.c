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
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "pqueue.h"

// --------------------------------------------------------------------------
// | Defines																|
// --------------------------------------------------------------------------

#define MAX_ITEMS 10
#define STRLEN 6

#define CHECK_COND(cond) if (__sync_bool_compare_and_swap(&cond, 1, 1)) break;
#define SWAP_COND(cond, a, b) while(1) { if (__sync_bool_compare_and_swap(&cond, a, b)) break; }

// --------------------------------------------------------------------------
// | Global static and volatile variables									|
// --------------------------------------------------------------------------

volatile unsigned int cond = 0, remaining_threads = 2;

static Priqueue *heap = NULL;
static sigset_t signal_mask;

// --------------------------------------------------------------------------
// | Helper functions														|
// --------------------------------------------------------------------------

void cleanup(int code) {
	printf("Waiting for all threads to finish...\n");

	while (remaining_threads > 0);

	printf("Freeing priqueue...\n");

	priqueue_free(heap);

	printf("Shutdown...\n");

	pthread_exit(NULL);

	exit(code);
}

int setup_signals() {
	sigemptyset(&signal_mask);

	if (sigaddset(&signal_mask, SIGINT) != 0) {
		printf("sigaddset() for SIGINT failed!\n");
		return -1;
	}

	if (sigaddset(&signal_mask, SIGTERM) != 0) {
		printf("sigaddset() for SIGTERM failed!\n");
		return -1;
	}

	pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);

	return 0;
}

int is_element_existing(Priqueue *h, char *lookval) {
	int ret = -1;

	Priqueue_Iterator *hit = priqueue_iterator_create(h);

	if (hit == NULL) {
		printf("Error while allocating memory for queue iterator!\n");
		return -2;
	}

	Node *node = NULL;

	while ((node = priqueue_iterator_get_next(hit)) != NULL) {
		char *buffer = (char *) node->data->data;

		if (strncmp(lookval, buffer, STRLEN) == 0) {
			ret = 0;
			break;
		}
	}

	priqueue_iterator_free(hit);

	return ret;
}

// --------------------------------------------------------------------------
// | Signal handler thread													|
// --------------------------------------------------------------------------

void *signal_thread(void *arg) {
	int err, sig_caught;

	for (;;) {
		err = sigwait(&signal_mask, &sig_caught);

		if (err != 0) {
			printf("sigwait failed\n");
			SWAP_COND(cond, 0, 1);
			cleanup(EXIT_FAILURE);
		}

		switch (sig_caught) {
			case SIGINT:
				printf("SIGINT received\n");
			case SIGTERM:
				printf("SIGTERM received\n");
				SWAP_COND(cond, 0, 1);
				cleanup(EXIT_SUCCESS);
				break;
			default:
				printf("Unexpected signal %d received\n", sig_caught);
				break;
		}
	}

	return NULL;
}

// --------------------------------------------------------------------------
// | Producer and consumer threads											|
// --------------------------------------------------------------------------

void *producer(void *arg) {
	Priqueue *h = (Priqueue *)arg;

	printf("Producer started!\n");

	unsigned int i = 1, j = 1;
	for (;;) {
		CHECK_COND(cond);

		for(i = 1; i < MAX_ITEMS; i++) {
			sched_yield();
			CHECK_COND(cond);

			Data *value = (Data *) malloc(sizeof(Data));
			value->data = (char *) malloc((STRLEN + 1) * sizeof(char));
			sprintf(value->data, "test %d", i);

			if (is_element_existing(h, value->data) != 0) {
				printf("--> Insert '%s' with priority %d\n", (char *) value->data, j);
				priqueue_insert(h, value, j);
				j++;
			} else {
				printf("!!! '%s' NOT inserted\n", (char *) value->data);
				free(value->data);
				free(value);
			}

			usleep(300 * 1000); // 300 ms
		}
	}

	remaining_threads--;

	return NULL;
}

void *consumer(void *arg) {
	Node *d = NULL;
	Priqueue *h = (Priqueue *)arg;

	printf("Consumer started!\n");

	for (;;) {
		sched_yield();
		CHECK_COND(cond);

		d = priqueue_pop(h);

		if (d != NULL) {
			printf("<== Remove '%s' with priority %lu (%d items remaining in queue)\n", 
				   (char *) d->data->data, d->priority, priqueue_getsize(h));
			priqueue_node_free(h, d);
			sleep(1); // 1 s
		}
	}
	
	remaining_threads--;

	return NULL;
}

int main() {
	pthread_t t1, t2, sigt;

	heap = priqueue_initialize(MAX_ITEMS, 0, 0);

	if (heap != NULL) {
		printf("Priqueue successfully initialized!\n");
	} else {
		printf("Error initializing Priqueue! Exit...\n");
		return EXIT_FAILURE;
	}

	if (setup_signals() != 0) {
		priqueue_free(heap);
		return EXIT_FAILURE;
	}

	pthread_create(&t1, NULL, producer, (void *) heap);
	pthread_create(&t2, NULL, consumer, (void *) heap);
	pthread_create(&sigt, NULL, signal_thread, NULL);

	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	pthread_join(sigt, NULL);

	return EXIT_SUCCESS;
}
