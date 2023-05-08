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

#define MAX_ELEMENTS 10 /// Given by file (read) CHANGE!!


int started = 0; //0 -> 'false', 1 -> 'true'
pthread_mutex_t mutex;
pthread_cond_t start;

pthread_cond_t notFull; 
pthread_cond_t notEmpty;

void *producer(void *param) {
	// Users through ATM 'produce' bank operations
	
	int id;
	int p; /// Change to struct element* elem;
	int pos = 0; ///
	
	// Producer thread has started:
	pthread_mutex_lock(&mutex);
	started = 1;
	id = *((int *)param);
	pthread_cond_signal(&start); // data has been copied
	pthread_mutex_unlock(&mutex);
	
	//Producing
	for (int i = 0; i < MAX_ELEMENTS; i++){
	
		p = i;
		printf("PRODUCER: %d, Petition_value: %d, ElementId: %d\n", id, i, p);
		
		// Critical section beggin:
		pthread_mutex_lock(&mutex);
		while (n_elements == MAX_ELEMENTS){ /// while queue is full
			pthread_cond_wait(&notFull, &mutex); // CHECK 
		}
		
		// Insert in circular buffer the produced element
		///call our queue instead
		buffer[pos] = p;
		pos = (pos + 1) % MAX_BUFFER;
		n_elements++;
		
		// New element produced warn and critical section end:
		pthread_cond_signal(&notEmpty); 
		pthread_mutex_unlock(&mutex);
	}
	
	
	// End producer execution
	printf("Producer: %d. End\n", id);
	pthread_exit(0);
	return NULL;

}

void *consumer(void *param) {
	// Workers 'consume' the ATM operations introduced by the producers to the circular queue
	
	int id;
	int p; /// Change to struct element* elem;
	int pos = 0; ///
	
	// Consumer thread has started:
	pthread_mutex_lock(&mutex);
	started = 1;
	id = *((int *)param);
	pthread_cond_signal(&start); // data has been copied
	pthread_mutex_unlock(&mutex);
	
	//Consuming
	for (int i = 0; ; i++){
		
		// Critical section beggin
		pthread_mutex_lock(&mutex);
		while (n_elements == 0){ // while queue is empty
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
		buffer[pos] = p; 
		pos = (pos + 1) % MAX_BUFFER; 
		n_elements--; 
		
		printf("CONSUMER: %d, Petition_value: %d, ElementId: %d\n", id, i, p);
		
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
 
int main (int argc, const char * argv[] ) {

	// {Read
	// ...
	// Read}
	
	void *retval;
	pthread_t threads[N_PRODUCERS + M_CONSUMERS];
	
	// initialize
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&start, NULL);
	pthread_cond_init(&notFull, NULL);
	pthread_cond_init(&notEmpty, NULL);
	
	// create PRODUCERS
	for (int i=0; i<N_PRODUCERS; i++) {
		pthread_create(&(threads[i]), NULL, producer, &i);
		
		pthread_mutex_lock(&mutex);
		while (!started) {
			pthread_cond_wait(&start, &mutex);
		}
		started = 0;
		pthread_mutex_unlock(&mutex);
	}
	
	// create CONSUMERS
	for (int i=0; i<M_CONSUMERS; i++) {
		pthread_create(&(threads[N_PRODUCERS + i]), NULL, consumer, &i);
		
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
	
	return 0;
}
