//OS-P3 2022-2023

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"



//To create a queue
queue* queue_init(int size){
	queue *cq = (queue*)malloc(sizeof(queue)); //allocate a cq in the heap
    cq->size = size;
    cq->data = (struct element*)malloc(size*sizeof(struct element));//allocate the array it holds as content attribute in the heap
    cq->head = -1; //by default, initially head and tail are both equal to -1
    cq->tail = -1; 
    return cq;
}

//To check queue state
int queue_empty(queue *cq){
	if(cq->head == -1 && cq->tail == -1) //by default this means that the queue is empty
    {
        return 1;
    }
    return 0;
}

int queue_full(queue *cq){
	if(queue_empty(cq))
    {
        return 0;
    }
    if((cq->tail +1)%cq->size == cq->head)
    {
        return 1;
    }
    return 0;
}

// To Enqueue an element
int queue_put(queue *cq, struct element *e) {
	if(!queue_full(cq)) //check first if the queue is full 
    {
        if(queue_empty(cq)) //then, in this special case, recall that we must set head to 0 since it is -1
        {
            cq->head = 0;
        }
        cq->data[(cq->tail+1)%cq->size] = *e; //then, insert it at tail+1 and update the value of tail
        cq->tail = (cq->tail +1)%cq->size;
        return 1;
    }
    return -1;
}


// To Dequeue an element.
struct element* queue_get(queue *cq) {
	if(!queue_empty(cq)) //check first if the queue is empty already
    {
        struct element *e = &cq->data[cq->head]; //element to drop off the queue
        if(cq->head == cq->tail) //only one element so after dequeing, set the head and tail to -1
        {
            cq->head = -1;
            cq->tail = -1;
            return e;
        }
        //if not the only one element then, increment modulo size the value of head
        cq->head = (cq->head+1)%cq->size;
        return e;
    }
    return NULL;
}

//To destroy the queue and free the resources
int queue_destroy(queue *cq){
	//first, free its content array from the heap and then the object pointer
    free(cq->data);
    free(cq);
	return 0;
}
