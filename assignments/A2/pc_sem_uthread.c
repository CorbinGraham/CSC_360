#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_sem.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;


int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

int items = 0;
uthread_sem_t items_sem;
//uthread_sem_t consumer_sem;

//I'm not totally sure if this is the correct way to do this
//however, it seems to work well.
void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    if(items < MAX_ITEMS) {
      uthread_sem_wait(items_sem);
      items++;
      histogram[items]++;
    }
    uthread_sem_signal(items_sem);
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    if(items > 0) {
      uthread_sem_wait(items_sem);
      items--;
      histogram[items]++;
    }
    uthread_sem_signal(items_sem);
  }
  return NULL;
}

int main (int argc, char** argv) {
  uthread_init(4);

  items_sem = uthread_sem_create(items);
  //consumer_sem = uthread_sem_create(items);
  //For my own understanding:
    //These lines START RUNNING THE METHODS.
    //Code will continue beyond WHILE these run.
  uthread_t producer1_var = uthread_create(producer, NULL);
  uthread_t producer2_var = uthread_create(producer, NULL);

  uthread_t consumer1_var = uthread_create(consumer, consumer);
  uthread_t consumer2_var = uthread_create(consumer, consumer);

  //Destroy/join the threads once they are done running the method
  uthread_join(producer1_var, NULL);
  uthread_join(producer2_var, NULL);

  uthread_join(consumer1_var, NULL);
  uthread_join(consumer2_var, NULL);

  // TODO: Create Threads and Join

  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  //assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
