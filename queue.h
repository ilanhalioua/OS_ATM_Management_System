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

int main(void)
{
    queue *Q = queue_init(10);
    int a = 0;
    struct element e;
    for(int i = 0; i<5; i++)
    {
        printf("%d element insertion: \n",i);
        scanf("%d account n# -> ",&a);
        e.account_number = a;
        queue_put(Q, &e);
    }
    printf(" Queue details : \n");
    printf("Head : %d, Tail : %d, Length : %d, Size : %d\n",Q->head, Q->tail, Q->length, Q->size);
    for(int j = 0; j<5; j++)
    {
        printf("%d Element's account number is %d\n", j+1, queue_get(Q)->account_number);
    }
    return 0;
}
