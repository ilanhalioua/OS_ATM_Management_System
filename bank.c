//OS-P3 2022-2023

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include "queue.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int num_ATMs;
int num_workers;

#define MAX_LINE_LENGTH 100 ///
int max_operations;

int end = 0;
struct queue *circular_queue;
struct element *list_client_ops;

int client_numop = 0; //counter of productions
int bank_numop = 0; //counter of consumptions

int started = 0; //0 -> 'false', 1 -> 'true'
pthread_mutex_t mutex;
pthread_cond_t start;

pthread_cond_t notFull; 
pthread_cond_t notEmpty;

float global_balance = 0.0;
float *account_balance ;


void *ATM(void *param) { // Producer thread
	// Users through ATM 'produce' bank operations
	
	int id;
	struct element elem;
	int amount_to_produce = *((int*)param);
	
	// Producer thread has started:
	if (pthread_mutex_lock(&mutex) < 0){
    		perror("Mutex error");
    		exit(-1);
  	}
	started = 1;
	id = *((int *)param);
	pthread_cond_signal(&start); // data has been copied
	if (pthread_mutex_unlock(&mutex) < 0){
    		perror("Mutex error");
    		exit(-1);
  	}
	
	//Producing
	while(client_numop < amount_to_produce) //producer(atm) must work while there are operations to produce
	{	
	
		/////printf("PRODUCER: %d, Petition_value: %d, ElementId: %d, Amount: %f\n", id, client_numop, elem.operation_id, elem.amount);
		
		// Critical section beggin:
		if (pthread_mutex_lock(&mutex) < 0){
    			perror("Mutex error");
    			exit(-1);
  		}
		while (queue_full(circular_queue)){ // while queue is full
			if (pthread_cond_wait(&notFull, &mutex) < 0){
				perror("Condition variable error");
    				exit(-1);
			}
		}
		
		elem = list_client_ops[client_numop];
		client_numop += 1;
		
		// Insert in circular buffer the produced element
		if(queue_put(circular_queue, &elem) < 0){
		      perror("Failed to enqueue an element");
		      exit(-1);
		}
		    
		// New element produced warn and critical section end:
		if(pthread_cond_signal(&notEmpty) < 0){
		      perror("Error while warning that queue is not empty");
		      exit(-1);
		}
		if (pthread_mutex_unlock(&mutex) < 0){
    			perror("Mutex error");
    			exit(-1);
  		}
	}
	
	
	// End producer execution
	/////printf("Producer: %d. End\n", id);
	pthread_exit(0);
	return NULL;

}

void *Worker(void *param) { // Consumer thread
	// Workers 'consume' the ATM operations introduced by the producers to the circular queue
	
	int id;
	struct element* data;
	int amount_to_consume = *((int*)param);
	
	// Consumer thread has started:
	pthread_mutex_lock(&mutex);
	started = 1;
	id = *((int *)param);
	pthread_cond_signal(&start); // data has been copied
	pthread_mutex_unlock(&mutex);
	
	//Consuming
	while (bank_numop < amount_to_consume) //each worker must consume as long as there are operations left to consume
	{

		// Critical section beggin
		if (pthread_mutex_lock(&mutex) < 0){
    			perror("Mutex error");
    			exit(-1);
  		}
		while (queue_empty(circular_queue)){ // while queue is empty
			if (end == 1) // Queue is empty and we have finished
			{
				/////printf("Consumer: %d. End\n", id);
				if (pthread_mutex_unlock(&mutex) < 0){
    					perror("Mutex error");
    					exit(-1);
  				}
				pthread_exit(0);
			}
			
			// Queue is empty but we have NOT finished
			if (pthread_cond_wait(&notEmpty, &mutex) < 0){
				perror("Condition variable error");
    				exit(-1);
			} // Sleep until queue is not empty
		}
		
		// Remove consumed element from circular buffer
		data = queue_get(circular_queue);	
		
		switch (data->operation_id) {
		    case 1: // CREATE
		      account_balance[data->account_number] = 0;
		      printf("%d CREATE %d BALANCE=%.0f TOTAL=%.0f\n", bank_numop+1, data->account_number, account_balance[data->account_number], global_balance);
		      break;
		    case 2: // DEPOSIT
		      account_balance[data->account_number] += data->amount;
		      global_balance += data->amount;
		      printf("%d DEPOSIT %d %.0f BALANCE=%.0f TOTAL=%.0f\n", bank_numop+1, data->account_number, data->amount, account_balance[data->account_number], global_balance);
		      break;
		    case 3: // TRANSFER
		      // Total stays the same (Balance of the accounts change)
		      account_balance[data->acc_from] -= data->amount;
		      account_balance[data->acc_to] += data->amount;
		      printf("%d TRANSFER %d %d %.0f BALANCE=%.0f TOTAL=%.0f\n", bank_numop+1, data->acc_from, data->acc_to, data->amount, account_balance[data->acc_to], global_balance);
		      break;
		    case 4: // WITHDRAW
		      account_balance[data->account_number] -= data->amount;
		      global_balance -= data->amount;
		      printf("%d WITHDRAW %d %.0f BALANCE=%.0f TOTAL=%.0f\n", bank_numop+1, data->account_number, data->amount, account_balance[data->account_number], global_balance);
		      break;
		    case 5: // BALANCE
		      // Nada creo
		      printf("%d BALANCE %d BALANCE=%.0f TOTAL=%.0f\n", bank_numop+1, data->account_number, account_balance[data->account_number], global_balance);
		      break;
		    default:
		      perror("Not valid operation");
		} 
		
		/////printf("CONSUMER: %d, Petition_value: %d, ElementId: %d\n", id, bank_numop, data->operation_id);
		bank_numop +=1;
		// Element consumed warn and critical section end:
		if (pthread_cond_signal(&notFull) < 0){
    			perror("Error while warning that queue is not full");
    			exit(-1);
  		}
		if (pthread_mutex_unlock(&mutex) < 0){
    			perror("Mutex error");
    			exit(-1);
  		}

	}
	
	// End producer execution
	/////printf("Consumer: %d. End\n", id);
	pthread_exit(0);
	return (void*)(long)1;
}
	

/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */
 
int fileLines(const char filename[]) {
  FILE *file = fopen(filename, "r");
  if(file == NULL){
    perror("Error opening file");
    exit(-1);
  }
  char chr;
  int lines = 0;
  while (!feof(file)) {
    chr = fgetc(file);
    if (chr == '\n') {
      lines++;
    }
  }
  fclose(file);
  return lines;
}
 
int main (int argc, const char * argv[] ) {

	// ./bank <file name> <num ATMs> <num workers> <max accounts> <buff size>	
	
	num_ATMs = atoi(argv[2]);
	num_workers = atoi(argv[3]);
	int max_accounts = atoi(argv[4]);
	int buff_size = atoi(argv[5]);
  	account_balance = (float*)malloc(max_accounts*sizeof(float));
	circular_queue = queue_init(buff_size);
	
	int nlines;
	
	if (argc != 6) {
    		perror("Invalid number of arguments\nUsage: ./bank <file name> <num ATMs> <num workers> <max accounts> <buff size>");
    		return -1;
  	}
  	
  	if (num_ATMs < 0 || num_workers < 0 || max_accounts < 0 || buff_size < 0) {
    		perror("Insert positive arguments please");
    		return -1;
	}
	

	
	// 1) READ FROM FILE
	
	FILE *fp = fopen(argv[1], "r");
	    if (fp == NULL) {
		perror("Error opening file");
		return -1;
	    }   
	    
	    //Falta lectura de la primera linea del file!! (Y control de errores)
	    if(fscanf(fp, "%d\n", &max_operations) < 0){
    		perror("Error retrieving data from input file");
    		exit(-1);
  	    }
	    /////printf("Number of Operations to be processed: %d\n\n", max_operations);
	    
	    list_client_ops = (struct element*)malloc(max_operations * sizeof(struct element)); //array of elements to be inserted by main process from the text file
	    
	    if (max_operations > 200) {
	    	perror("Maximum number of operations is: 200");
    		return -1;
	    }
	    
	    nlines = fileLines(argv[1]);
	    /////printf("max_operations:%d vs nlines - 1: %d\n", max_operations, nlines-1);
	    if (nlines - 1 != max_operations) 
	    {
	        perror("Number of operations in file should match with the value in the first line");
    		return -1;
	    } 

	    char line[MAX_LINE_LENGTH];
	    int i = 0;
	    
	    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
	    	// Initialize amount to 0 for each new element
        	list_client_ops[i].amount = 0.0;
	    
		char *token = strtok(line, " ");
		if (strcmp(token, "CREATE") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 1;
		    list_client_ops[i].account_number = atoi(token);
		    /////printf("Element: list_client_ops[%d] -> opId = %d (CREATE), accNum = %d\n",i, list_client_ops[i].operation_id, list_client_ops[i].account_number);
		} else if (strcmp(token, "DEPOSIT") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 2;
		    list_client_ops[i].account_number = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].amount = atof(token);
		    /////printf("Element: list_client_ops[%d] -> opId = %d (DEPOSIT), accNum = %d, amount = %.2f\n",i, list_client_ops[i].operation_id, list_client_ops[i].account_number, list_client_ops[i].amount);
		} else if (strcmp(token, "TRANSFER") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 3;
		    list_client_ops[i].acc_from = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].acc_to = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].amount = atof(token);
		    /////printf("Element: list_client_ops[%d] -> opId = %d (TRANSFER), acc_from = %d, acc_to = %d, amount = %.2f\n",i, list_client_ops[i].operation_id, list_client_ops[i].acc_from, list_client_ops[i].acc_to, list_client_ops[i].amount);
		} else if (strcmp(token, "WITHDRAW") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 4;
		    list_client_ops[i].account_number = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].amount = atof(token);
		    /////printf("Element: list_client_ops[%d] -> opId = %d (WITHDRAW), accNum = %d, amount = %.2f\n",i, list_client_ops[i].operation_id, list_client_ops[i].account_number, list_client_ops[i].amount);
		} else if (strcmp(token, "BALANCE") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 5;
		    list_client_ops[i].account_number = atoi(token);
		    /////printf("Element: list_client_ops[%d] -> opId = %d (BALANCE), accNum = %d\n",i, list_client_ops[i].operation_id, list_client_ops[i].account_number);
		}
		i++;
	    }

	    fclose(fp);
	    
	// FINISH READING
	
	// ------ delete
	/////printf("\n\n");
	/////printf("ElementId Leyend:\n1 -> 'CREATE', 2 -> 'DEPOSIT', 3 -> 'TRANSFER', 4 -> 'WITHDRAW', 5 -> 'BALANCE'\n");
	/////printf("\n");
	// ------ delete
	
	void *retval;
	pthread_t threads[num_ATMs + num_workers];
	
	// initialize
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&start, NULL);
	pthread_cond_init(&notFull, NULL);
	pthread_cond_init(&notEmpty, NULL);
	
	// create PRODUCERS
	for (int i=0; i<num_ATMs; i++) {
		pthread_create(&(threads[i]), NULL, ATM, &max_operations);
		
		pthread_mutex_lock(&mutex);
		while (!started) {
			pthread_cond_wait(&start, &mutex);
		}
		started = 0;
		pthread_mutex_unlock(&mutex);
	}
	
	// create CONSUMERS
	for (int i=0; i<num_workers; i++) {
	        pthread_create(&(threads[num_ATMs + i]), NULL, Worker, &max_operations);
		
		pthread_mutex_lock(&mutex);
		while (!started) {
			pthread_cond_wait(&start, &mutex);
		}
		started = 0;
		pthread_mutex_unlock(&mutex);
	}
	
	// wait PRODUCERS exectution end
	for (int i=0; i<num_ATMs; i++) {
		pthread_join(threads[i], &retval);
	}
	
	// CONSUMERS get the signal when PRODUCERS exectution end
	pthread_mutex_lock(&mutex);
	end = 1;
	pthread_cond_broadcast(&notEmpty);
	pthread_mutex_unlock(&mutex);
	
	// wait CONSUMERS exectution end
	for (int i=0; i<num_workers; i++) {
		pthread_join(threads[num_ATMs + i], &retval);
	}
	
	// destroy
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&start);
	pthread_cond_destroy(&notFull);
	pthread_cond_destroy(&notEmpty);
	
	free(list_client_ops);
	
	return 0;
}
