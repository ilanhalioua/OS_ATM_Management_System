//OS-P3 2022-2023

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "queue.h"




struct CircularQueue
{
    int head; //index for the first element in the queue
    int tail; //index for the last element in the queue
    int size; // fixed amount of data capable of storing
    int *data; // actual array of length size of data to store
};

typedef struct CircularQueue CQ; //shortcut to refer to the type struct CircularQueue


CQ *cq_init(int s); //circular queue constructor
void cq_destroy(CQ *cq) //circular queue structure destructor
{
    //first, free its content array from the heap and then the object pointer
    free(cq->data);
    free(cq);
}
int cq_empty(CQ *cq); //checks if the queue is empty
int cq_full(CQ *cq); //check if the queue is full
int cq_enqueue(CQ *cq, int e); //circular queue function to enqueue data
int *cq_dequeue(CQ *cq); //circular queue function to dequeue data
void cq_show(CQ *cq) //prints the order of the cqueue on screen
{
    for(int i = 0; i<cq->size; i++)
    {
        printf("%d\t",cq->data[i]);
    }
}

int main()
{

    printf("Enter the size of the circular queue to construct here : ");
    int s;
    scanf("%d",&s);
    CQ *mycq;
    mycq = cq_init(s);
    int ask; //1 for inserting, 0 for removing from the cqueue mycq
    printf("Insert(1) or remove(0) : ");
    scanf("%d",&ask);
    int d;
    int *val;
    while(ask == 1 || ask == 0)
    {
        if(ask)
        {
            printf("Element to insert-> ");
            scanf("%d",&d);
            if(cq_enqueue(mycq, d) == -1)
            {
                printf("Cannot enqueue!The queue is full at the moment\n");
            }
            else
            {
                printf("Element was added succesfully!\n");
            }
        }
        else
        {
            int *val = cq_dequeue(mycq);
            if(val == NULL)
            {
                printf("Cannot dequeue!The queue is empty at the moment\n");
            }
            else
            {
                printf("Dequed element -> %d\n",*val);
            }
        }
        cq_show(mycq);
        printf("Insert(1) or remove(0) : ");
        scanf("%d",&ask);
        
    }
    cq_destroy(mycq); 
    printf("The circular queue has been destroyed !\n");

    return 0;
}
 CQ *cq_init(int s)
{
    CQ *cq = (CQ*)malloc(sizeof(CQ)); //allocate a cq in the heap
    cq->size = s;
    cq->data = (int*)malloc(s*sizeof(int));//allocate the array it holds as content attribute in the heap
    cq->head = -1; //by default, initially head and tail are both equal to -1
    cq->tail = -1; 
    return cq;
}

int cq_empty(CQ *cq)
{
    if(cq->head == -1 && cq->tail == -1) //by default this means that the queue is empty
    {
        return 1;
    }
    return 0;
}

int cq_full(CQ *cq)
{
    if(cq_empty(cq))
    {
        return 0;
    }
    if((cq->tail +1)%cq->size == cq->head)
    {
        return 1;
    }
    return 0;
}



int cq_enqueue(CQ *cq, int e)
{
    if(!cq_full(cq)) //check first if the queue is full 
    {
        if(cq_empty(cq)) //then, in this special case, recall that we must set head to 0 since it is -1
        {
            cq->head = 0;
        }
        cq->data[(cq->tail+1)%cq->size] = e; //then, insert it at tail+1 and update the value of tail
        cq->tail = (cq->tail +1)%cq->size;
        return 1;
    }
    return -1;
}

int *cq_dequeue(CQ *cq)
{
    if(!cq_empty(cq)) //check first if the queue is empty already
    {
        int *e = &cq->data[cq->head]; //element to drop off the queue
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
