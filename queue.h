#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
	// Define the struct yourself
	int account_number;
	int acc_from;
	int acc_to;
	float amount;
	int operation_id; // 1 if op is 'CREATE', 2 if op is 'DEPOSIT', 3 if op is 'TRANSFER', 4 if op is 'WITHDRAW', 5 if op is 'BALANCE'
};

typedef struct queue {
    int head; //index for the first element in the queue
    int tail; //index for the last element in the queue
    int size; // fixed amount of data capable of storing
    struct element *data; // actual array of length size of element type 
}queue;

queue* queue_init (int size);
int queue_empty (queue *cq);
int queue_full(queue *cq);
int queue_put (queue *cq, struct element* e);
struct element * queue_get(queue *cq);
int queue_destroy (queue *cq);

#endif
