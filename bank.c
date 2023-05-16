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

//global variables and constants:
#define MAX_OPS 200

int num_ATMs;
int num_workers;

int max_operations;

int end = 0;
struct queue *circular_queue; //circular queue (buffer) shared by producers and consumer threads
struct element *list_client_ops; //dynamic array of operations 

int client_numop = 0; //counter of productions
int bank_numop = 0; //counter of consumptions

int started = 0; //0 -> 'false', 1 -> 'true'
pthread_mutex_t mutex; //mutex lock to synchronize threads
pthread_cond_t start; 

pthread_cond_t notFull; //condition variable on the queue being not full 
pthread_cond_t notEmpty; //condition variable on the queue being not empty 

float global_balance = 0.0; //total balance of the bank
float *account_balance ; //dynamic array of integers (of size = <max_account>) that stores the ith account current balance at ith index


void *ATM(void *param) { // Producer thread
	// Users through ATM 'produce' bank operations
	
	int id;
	struct element elem;
	int amount_to_produce = *((int*)param); //typecast the void pointer params and dereference it in order to obtain the actual number of operations to process (the one in the first line of the file)
	
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
		
		// Critical section beggin:
		if (pthread_mutex_lock(&mutex) < 0){
    			perror("Mutex error");
    			exit(-1);
  		}
		while (queue_full(circular_queue)){ // while queue is full
			if (pthread_cond_wait(&notFull, &mutex) < 0){ //waits until signal indicating that queue is not full (releases the mutex)
				perror("Condition variable error");
    				exit(-1);
			}
		}
		
		elem = list_client_ops[client_numop]; 
		client_numop += 1; //increment by 1 the number of productions done
		
		// Insert in circular buffer the produced element
		if(queue_put(circular_queue, &elem) < 0){ //enqueue the operation from the array at index client_numop
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
	pthread_exit(0);
	return NULL;

}

void *Worker(void *param) { // Consumer thread
	// Workers 'consume' the ATM operations introduced by the producers to the circular queue
	
	int id;
	struct element* data;
	int amount_to_consume = *((int*)param); //typecast the void pointer params as an int pointer and dereference it to access the number of ops to process (the one in the first line of the input file)
	
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
		data = queue_get(circular_queue); //dequeue the operation on the circular queue	
		
		//take the respective action and print info on screen
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
		
		bank_numop +=1; //increment by 1 the number of consumptions done
		
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
	/* NOTE: The program is executed on the following way:
	    ./bank <file name> <num_ATMs> <num_workers> <max_accounts> <buff_size>
	*/	
	
	//store the input data inside variables for readability
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
	

	
	// 1) READ FROM FILE (and store operations inside the array of type elements: list_client_ops[])
	
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
	    
	    list_client_ops = (struct element*)malloc(max_operations * sizeof(struct element)); //array of elements to be inserted by main process from the text file
	    
	    if (max_operations > MAX_OPS) {
	    	perror("Maximum number of operations is: 200");
    		return -1;
	    }
	    
	    nlines = fileLines(argv[1]);
	    if (nlines - 1 != max_operations) 
	    {
	        perror("Number of operations in file should match with the value in the first line");
    		return -1;
	    } 

	    char line[100];
	    int i = 0;
	    
	    while (fgets(line, 100, fp) != NULL) {
	    	// Initialize amount to 0 for each new element
        	list_client_ops[i].amount = 0.0;
	    
		char *token = strtok(line, " ");
		if (strcmp(token, "CREATE") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 1;
		    list_client_ops[i].account_number = atoi(token);
		} else if (strcmp(token, "DEPOSIT") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 2;
		    list_client_ops[i].account_number = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].amount = atof(token);
		} else if (strcmp(token, "TRANSFER") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 3;
		    list_client_ops[i].acc_from = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].acc_to = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].amount = atof(token);
		} else if (strcmp(token, "WITHDRAW") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 4;
		    list_client_ops[i].account_number = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].amount = atof(token);
		} else if (strcmp(token, "BALANCE") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 5;
		    list_client_ops[i].account_number = atoi(token);
		}
		i++;
	    }

	    fclose(fp);
	    
	// FINISH READING
	
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
