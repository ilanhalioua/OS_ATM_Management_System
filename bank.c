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

#define N_PRODUCERS 16
#define M_CONSUMERS 5

#define MAX_BUFFER 8 ///
int buffer[MAX_BUFFER]; ///
int n_elements = 0; ///

#define MAX_ELEMENTS 16 /// Given by file (read) CHANGE!!


int started = 0; //0 -> 'false', 1 -> 'true'
pthread_mutex_t mutex;
pthread_cond_t start;

pthread_cond_t full;
pthread_cond_t empty;

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
		
		pthread_mutex_lock(&mutex);
		while (n_elements == MAX_ELEMENTS){ /// while queue is full
			pthread_cond_wait(&full, NULL); // CHECK 
		}
		p = i;
		printf("PRODUCER: %d, Petition_value: %d, ElementId: %d\n", id, i, p);
		
		///call my queue
		buffer[pos] = p;
		pos = (pos + 1) % MAX_BUFFER;
		n_elements++;
		
		pthread_cond_signal(&full);
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
		
		pthread_mutex_lock(&mutex);
		while (n_elements == 0){ // while queue is empty
			pthread_signal_wait(&empty, NULL); // CHECK 
		}
		///call my queue
		//p = myQueue;
		
		//printf("CONSUMER: %d, Petition_value: %d, ElementId: %d\n", id, i, p);
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);

	}
	
	// End producer execution
	printf("Consumer: %d. End\n", id);
	pthread_exit(0);
	return NULL;
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

	pthread_t threads[N_PRODUCERS + M_CONSUMERS];
	
	// initialize
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&start, NULL);
	
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
		pthread_join(threads[i], NULL);
	}
	
	// CONSUMERS get the signal when PRODUCERS exectution end
	
	// wait CONSUMERS exectution end
	for (int i=0; i<M_CONSUMERS; i++) {
		pthread_join(threads[N_PRODUCERS + i], NULL);
	}
	
	// destroy
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&start);
	
	return 0;
}
