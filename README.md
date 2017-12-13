# priqueue

Thread Safe Priority Queue in C

# Example

see *test.c*

# API

Initialize Priority Queue with *initial_length* and choose if its functions should be *blocking* or not. On failure, *priqueue_initialize* returns NULL.

    Priqueue *priqueue_initialize(unsigned int initial_length, unsigned int blocking);

Insert into Queue with specified priority.

    void priqueue_insert(Priqueue *, Data *, uintptr_t);

Pop the node with maximum priority ( head ) . 

    Node *priqueue_pop(Priqueue *);

Deallocate data node after poping.

    void priqueue_node_free(Priqueue *, Node *);

Deallocate memory. Also deallocates remaining nodes from queue .

    void priqueue_free(Priqueue *);
	
Create a new queue from all elements in current queue .

    Priqueue* priqueue_popall(Priqueue *heap);

# ToDos

1. [Lightweight Mutex](http://preshing.com/20120226/roll-your-own-lightweight-mutex/)
2. API to shrink array size.
