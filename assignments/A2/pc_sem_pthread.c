#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;


int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;
sem_t items_sem;
//uthread_sem_t consumer_sem;

//I'm not totally sure if this is the correct way to do this
//however, it seems to work well.
void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    int buffer;
    sem_getvalue(&items_sem, &buffer);
    if(buffer < MAX_ITEMS) {
      sem_post(&items_sem);
      histogram[items]++;
    }
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    int buffer;
    sem_getvalue(&items_sem, &buffer);
    if(buffer > 0) {
      sem_wait(&items_sem);
      histogram[items]++;
    }
  }
  return NULL;
}

int main (int argc, char** argv) {
  sem_init(&items_sem, 0, items);
  pthread_t producer1_var;
  pthread_t producer2_var; 
  
  pthread_t consumer1_var;
  pthread_t consumer2_var;
  
  //For my own understanding:
    //These lines START RUNNING THE METHODS.
    //Code will continue beyond WHILE these run.
  pthread_create(&producer1_var, NULL, producer, NULL);
  pthread_create(&producer2_var, NULL, producer, NULL);

  pthread_create(&consumer1_var, NULL, consumer, NULL);
  pthread_create(&consumer2_var, NULL, consumer, NULL);

  //Destroy/join the threads once they are done running the method
  pthread_join(producer1_var, NULL);
  pthread_join(producer2_var, NULL);

  pthread_join(consumer1_var, NULL);
  pthread_join(consumer2_var, NULL);

  // TODO: Create Threads and Join

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  //assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
