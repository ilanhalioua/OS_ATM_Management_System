//OS-P3 2022-2023

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"



//To create a queue
queue* queue_init(int size){

	queue * q = (queue *)malloc(sizeof(queue));
	q -> data = malloc(size * sizeof(struct element));
	q -> head = 0;
	q -> tail = 0;
	q -> length = 0;
	q -> size = size;

	return q;
}


// To Enqueue an element
int queue_put(queue *q, struct element* x) {
	if (queue_full(q) == 0) {  // If queue is not full
		q -> tail = (q -> tail + 1) % q -> size;
		q -> data[q -> tail] = *x;
		q -> length++;
		return 0;
	}
	return -1;
}


// To Dequeue an element.
struct element* queue_get(queue *q) {
	struct element* element;

	if (queue_empty(q) == 0) {  // If queue is not empty
		element = &(q -> data[q -> head]);
		q -> head = (q -> head + 1) % q -> size;
		q -> length--;
	}
	return element;
}


//To check queue state
int queue_empty(queue *q){
	if (q -> length == 0){
		return 1;
	}
	return 0;
}

int queue_full(queue *q){
	if (q -> length == q -> size){
		return 1;
	}
	return 0;
}

//To destroy the queue and free the resources
int queue_destroy(queue *q){
	free(q);
	return 0;
}
