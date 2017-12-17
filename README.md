# priqueue

Thread Safe Priority Queue in C

The implementation is based on a max-heap derived from [here](http://www.eecs.wsu.edu/~ananth/CptS223/Lectures/heaps.pdf).

# Example

see *test.c*

# API

**WARNING:** Do not use the *priqueue_insert* and *priqueue_pop* functions outside of a running thread!

Initialize Priority Queue with *initial_length*, choose if its functions should be *blocking* or not and set a *limit_length* (zero means no limit). On failure, *priqueue_initialize* returns NULL.

    Priqueue *priqueue_initialize(unsigned int initial_length, unsigned int blocking, unsigned int limit_length);

Insert into Queue with specified priority. Returns 0 on success or -1 if queue is full (only possible if *limit_length* was set to a value greater than zero on *priqueue_initialize*).

    int priqueue_insert(Priqueue *, Data *, uintptr_t);

Pop the node with maximum priority (head). 

    Node *priqueue_pop(Priqueue *);

Deallocate data node after poping.

    void priqueue_node_free(Priqueue *, Node *);

Deallocate memory. Also deallocates remaining nodes from queue.

    void priqueue_free(Priqueue *);
	
Create a new queue from all elements in current queue.

    Priqueue* priqueue_popall(Priqueue *heap);

# ToDos

1. [Lightweight Mutex](http://preshing.com/20120226/roll-your-own-lightweight-mutex/)
2. API to shrink array size
