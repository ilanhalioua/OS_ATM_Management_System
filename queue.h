#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
	// Define the struct yourself
	int account_number;
	float amount;
	int idOperation;
};


struct CircularQueue
{
    int head; //index for the first element in the queue
    int tail; //index for the last element in the queue
    int size; // fixed amount of data capable of storing
    int *data; // actual array of length size of data to store
};

typedef struct CircularQueue CQ; //shortcut to refer to the type struct CircularQueue

CQ *cq_init(int s);
int cq_destroy(CQ *cq);
int cq_enqueue(CQ *q, struct element* elem);
struct element * cq_dequeue(CQ *cq);
int cq_empty(CQ *cq);
int cq_full(CQ *cq);

#endif
