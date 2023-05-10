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

#define N_PRODUCERS 2
#define M_CONSUMERS 1

#define MAX_BUFFER 5 ///
int buffer[MAX_BUFFER]; ///
int n_elements = 0; ///
int fin = 0;
struct queue *circular_queue;

#define MAX_ELEMENTS 10 /// Given by file (read) CHANGE!! -> num_ops

// ./bank <file name> <num ATMs> <num workers> <max accounts> <buff size>
// initialize balance[<max accounts>]!!!

int started = 0; //0 -> 'false', 1 -> 'true'
pthread_mutex_t mutex;
pthread_cond_t start;

pthread_cond_t notFull; 
pthread_cond_t notEmpty;

int total = 0;

void *ATM(void *param) { // Producer thread
	// Users through ATM 'produce' bank operations
	
	int id;
	int p; /// delete
	///struct element *elem;
	int pos = 0; /// delete
	
	// Producer thread has started:
	pthread_mutex_lock(&mutex);
	started = 1;
	id = *((int *)param);
	pthread_cond_signal(&start); // data has been copied
	pthread_mutex_unlock(&mutex);
	
	//Producing
	for (int client_numops = 0; client_numops < MAX_ELEMENTS; client_numops++){ //Change MAX_ELEMENTS to num_ops (given by line 1 of file.txt)
	
		p = client_numops; /// delete
		/// elem = list_client_ops[client_numops];
		printf("PRODUCER: %d, Petition_value: %d, ElementId: %d\n", id, client_numops, p); /// delete
		/// printf("PRODUCER: %d, Petition_value: %d, ElementId: %d\n", id, client_numops, elem.operation_id);
		
		// Critical section beggin:
		pthread_mutex_lock(&mutex);
		while (n_elements == MAX_ELEMENTS){ /// while queue is full <queue_full(circular_queue)>
			pthread_cond_wait(&notFull, &mutex);
		}
		
		// Insert in circular buffer the produced element
		///call our queue instead
		///queue_put(circular_queue, elem);
		buffer[pos] = p; /// delete
		pos = (pos + 1) % MAX_BUFFER; /// delete
		n_elements++; /// delete
		
		// New element produced warn and critical section end:
		pthread_cond_signal(&notEmpty); 
		pthread_mutex_unlock(&mutex);
	}
	
	
	// End producer execution
	printf("Producer: %d. End\n", id);
	pthread_exit(0);
	return NULL;

}

void *Worker(void *param) { // Consumer thread
	// Workers 'consume' the ATM operations introduced by the producers to the circular queue
	
	int id;
	int p; /// delete
	///struct element* data;
	int pos = 0; /// delete
	
	// Consumer thread has started:
	pthread_mutex_lock(&mutex);
	started = 1;
	id = *((int *)param);
	pthread_cond_signal(&start); // data has been copied
	pthread_mutex_unlock(&mutex);
	
	//Consuming
	for (int worker_numops = 0; ; worker_numops++){
		
		// Critical section beggin
		pthread_mutex_lock(&mutex);
		while (n_elements == 0){ // while queue is empty <queue_empty(circular_queue)>
			if (fin == 1) // Queue is empty and we have finished
			{
				printf("Consumer: %d. End\n", id);
				pthread_mutex_unlock(&mutex);
				pthread_exit(0);
			}
			
			// Queue is empty but we have NOT finished
			pthread_cond_wait(&notEmpty, &mutex); // Sleep until queue is not empty
		}
		
		// Remove consumed element from circular buffer
		///call our queue instead
		///data = queue_get(circular_queue); 
		p = worker_numops; /// delete
		buffer[pos] = p; /// delete
		pos = (pos + 1) % MAX_BUFFER; /// delete
		n_elements--; /// delete
		
		
		/* 
		switch (data->operation_id) {
		    case 1: // CREATE
		      balance[data->account_number] = 0;
		      //printf("%d CREATE %d BALANCE=%d TOTAL=%d\n", worker_numops, data->account_number, balance[data->account_number], total);
		      break;
		    case 2: // DEPOSIT
		      balance[data->account_number] = += data->amount;
		      total += data->amount;
		      //printf("%d DEPOSIT %d %d BALANCE=%d TOTAL=%d\n", worker_numops, data->account_number, data->amount, balance[data->account_number], total);
		      break;
		    case 3: // TRANSFER
		      // Total stays the same (Balance of the accounts change)
		      balance[data->acc_from] = -= data->amount;
		      balance[data->acc_to] = += data->amount;
		      //printf("%d TRANSFER %d %d %d BALANCE=%d TOTAL=%d\n", worker_numops, data->acc_from, data->acc_to, data->amount, balance[data->account_number], total);
		      break;
		    case 4: // WITHDRAW
		      balance[data->account_number] = -= data->amount;
		      total -= data->amount;
		      //printf("%d WITHDRAW %d %d BALANCE=%d TOTAL=%d\n", worker_numops, data->account_number, data->amount, balance[data->account_number], total);
		      break;
		    case 5: // BALANCE
		      // Nada creo
		      //printf("%d BALANCE %d BALANCE=%d TOTAL=%d\n", worker_numops, data->account_number, balance[data->account_number], total);
		      break;
		    default:
		      perror("Not valid operation");
		}*/ 
		
		printf("CONSUMER: %d, Petition_value: %d, ElementId: %d\n", id, worker_numops, p); /// delete 
		///printf("CONSUMER: %d, Petition_value: %d, ElementId: %d\n", id, worker_numops, data->operation_id);
		// Element consumed warn and critical section end:
		pthread_cond_signal(&notFull);
		pthread_mutex_unlock(&mutex);

	}
	
	// End producer execution
	printf("Consumer: %d. End\n", id);
	pthread_exit(0);
	return (void*)(long)1;
}
	

/**
 * Entry point
 * @param argc
 * @param argv
 * @return
 */
 
#define MAX_LINE_LENGTH 100 ///
#define MAX_OPS 200 ///


int num_ops;
 
int main (int argc, const char * argv[] ) {
        
        struct element *list_client_ops = (struct element*)malloc(MAX_OPS * sizeof(struct element)); //array of elements to be inserted by main process from the text file
	
	// 1) READ FROM FILE
	
	FILE *fp = fopen("file.txt", "r");
	    if (fp == NULL) {
		perror("Error opening file");
		return -1;
	    }
	    
	    //Falta lectura de la primera linea del file!! (Y control de errores)
	    fscanf(fp, "%d\n", &num_ops);
	    printf("Number of Operations to be processed: %d\n\n", num_ops);

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
		    printf("Element: list_client_ops[%d] -> opId = %d (CREATE), accNum = %d\n",i, list_client_ops[i].operation_id, list_client_ops[i].account_number);
		} else if (strcmp(token, "DEPOSIT") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 2;
		    list_client_ops[i].account_number = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].amount = atof(token);
		    printf("Element: list_client_ops[%d] -> opId = %d (DEPOSIT), accNum = %d, amount = %.2f\n",i, list_client_ops[i].operation_id, list_client_ops[i].account_number, list_client_ops[i].amount);
		} else if (strcmp(token, "TRANSFER") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 3;
		    list_client_ops[i].acc_from = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].acc_to = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].amount = atof(token);
		    printf("Element: list_client_ops[%d] -> opId = %d (TRANSFER), acc_from = %d, acc_to = %d, amount = %.2f\n",i, list_client_ops[i].operation_id, list_client_ops[i].acc_from, list_client_ops[i].acc_to, list_client_ops[i].amount);
		} else if (strcmp(token, "WITHDRAW") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 4;
		    list_client_ops[i].account_number = atoi(token);
		    token = strtok(NULL, " ");
		    list_client_ops[i].amount = atof(token);
		    printf("Element: list_client_ops[%d] -> opId = %d (WITHDRAW), accNum = %d, amount = %.2f\n",i, list_client_ops[i].operation_id, list_client_ops[i].account_number, list_client_ops[i].amount);
		} else if (strcmp(token, "BALANCE") == 0) {
		    token = strtok(NULL, " ");
		    list_client_ops[i].operation_id = 5;
		    list_client_ops[i].account_number = atoi(token);
		    printf("Element: list_client_ops[%d] -> opId = %d (BALANCE), accNum = %d\n",i, list_client_ops[i].operation_id, list_client_ops[i].account_number);
		}
		i++;
	    }

	    fclose(fp);
	    
	// FINISH READING
	
	// ------ delete
	printf("\n\n");
	// ------ delete
	
	void *retval;
	pthread_t threads[N_PRODUCERS + M_CONSUMERS];
	
	// initialize
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&start, NULL);
	pthread_cond_init(&notFull, NULL);
	pthread_cond_init(&notEmpty, NULL);
	
	// create PRODUCERS
	for (int i=0; i<N_PRODUCERS; i++) {
		pthread_create(&(threads[i]), NULL, ATM, &i);
		
		pthread_mutex_lock(&mutex);
		while (!started) {
			pthread_cond_wait(&start, &mutex);
		}
		started = 0;
		pthread_mutex_unlock(&mutex);
	}
	
	// create CONSUMERS
	for (int i=0; i<M_CONSUMERS; i++) {
		pthread_create(&(threads[N_PRODUCERS + i]), NULL, Worker, &i);
		
		pthread_mutex_lock(&mutex);
		while (!started) {
			pthread_cond_wait(&start, &mutex);
		}
		started = 0;
		pthread_mutex_unlock(&mutex);
	}
	
	// wait PRODUCERS exectution end
	for (int i=0; i<N_PRODUCERS; i++) {
		pthread_join(threads[i], &retval);
	}
	
	// CONSUMERS get the signal when PRODUCERS exectution end
	pthread_mutex_lock(&mutex);
	fin = 1;
	pthread_cond_broadcast(&notEmpty);
	pthread_mutex_unlock(&mutex);
	
	// wait CONSUMERS exectution end
	for (int i=0; i<M_CONSUMERS; i++) {
		pthread_join(threads[N_PRODUCERS + i], &retval);
	}
	
	// destroy
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&start);
	pthread_cond_destroy(&notFull);
	pthread_cond_destroy(&notEmpty);
	
	free(list_client_ops);
	
	return 0;
}
