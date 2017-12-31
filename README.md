# priqueue

Thread Safe Priority Queue in C

The implementation is based on a max-heap derived from [here](http://www.eecs.wsu.edu/~ananth/CptS223/Lectures/heaps.pdf).

# Example

see *test.c*

# API

**WARNING:** Do not use the *priqueue_insert* and *priqueue_pop* functions outside of a running thread!

## Generic functions

Initialize Priority Queue with *initial_length*, choose if its functions should be *blocking* or not and set a *limit_length* (zero means no limit). On failure, *priqueue_initialize* returns NULL.

    Priqueue *priqueue_initialize(unsigned int initial_length, unsigned int blocking, unsigned int limit_length);

Insert into Queue with specified priority. On success, *priqueue_insert* returns 0 or -1 if the queue is full (only possible if *limit_length* was set to a value greater than zero on *priqueue_initialize*).

    int priqueue_insert(Priqueue *, Data *, uintptr_t);

Pop the node with maximum priority (head). 

    Node *priqueue_pop(Priqueue *);

Deallocate data node after poping.

    void priqueue_node_free(Priqueue *, Node *);

Deallocate memory. Also deallocates remaining nodes from queue.

    void priqueue_free(Priqueue *);

Create a new queue from all elements in current queue.

    Priqueue *priqueue_popall(Priqueue *heap);

Check if queue is empty. If the queue is empty, *priqueue_isempty* returns 0 or -1 if the queue is not empty.

    int priqueue_isempty(Priqueue *);

Get the node with maximum priority (head) without removing it from the queue. If the queue is empty, *priqueue_peek* returns NULL.

    Node *priqueue_peek(Priqueue *);

## Iterator functions

Initialize a Priority Queue Iterator for a given Priority Queue. On failure, *priqueue_iterator_create* returns NULL.

    Priqueue_Iterator *priqueue_iterator_create(Priqueue *);

Returns the next node from the given Priority Queue Iterator. If the end of the queue is reached, *priqueue_iterator_get_next* returns NULL.

**Beware:** This function does not return the nodes in the correct order of the queue!

    Node *priqueue_iterator_get_next(Priqueue_Iterator *);

Deallocate the memory of the given Priority Queue Iterator. On success, *priqueue_iterator_free* returns 0 or -1 if the operation failed.

    int priqueue_iterator_free(Priqueue_Iterator *);

# ToDos

1. [Lightweight Mutex](http://preshing.com/20120226/roll-your-own-lightweight-mutex/)
2. API to shrink array size
