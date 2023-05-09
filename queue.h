#ifndef HEADER_FILE
#define HEADER_FILE


struct element {
	// Define the struct yourself
	int account_number;
	float amount;
	int idOperation;
};

typedef struct queue {
	// Define the struct yourself
	struct element *data;
	int head;
	int tail;
	int length;
	int size;
}queue;

queue* queue_init (int size);
int queue_destroy (queue *q);
int queue_put (queue *q, struct element* elem);
struct element * queue_get(queue *q);
int queue_empty (queue *q);
int queue_full(queue *q);

#endif
