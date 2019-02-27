#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

//Create both the mutex and condition
uthread_mutex_t  mutex;
uthread_cond_t  consumer_condition;
uthread_cond_t  producer_condition;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
// MAKE 2 CONDITION VARIABLES TO LET THE THREADS KNOW IF THE CAN PRODUCE OR CONSUME

int items = 0;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    uthread_mutex_lock(mutex);
    while(items >= MAX_ITEMS) {
      uthread_cond_wait(producer_condition);
      producer_wait_count++;
    }
    items++;
    histogram[items]++;
    uthread_cond_signal(consumer_condition);
    uthread_mutex_unlock(mutex);
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    uthread_mutex_lock(mutex);
    while(items <= 0) {
      uthread_cond_wait(consumer_condition);
      consumer_wait_count++;
    }
    items--;
    histogram[items]++;
    uthread_cond_signal(producer_condition);
    uthread_mutex_unlock(mutex);
    // TODO
  }
  return NULL;
}

int main (int argc, char** argv) {
  //Moved initialization of mutex/conds to main
  mutex = uthread_mutex_create();
  consumer_condition = uthread_cond_create(mutex);
  producer_condition = uthread_cond_create(mutex);

  uthread_init(4);
  
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
  
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  printf ("  sum=%d\n", sum);
}
