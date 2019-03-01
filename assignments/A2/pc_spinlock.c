#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"
#include "spinlock.h"

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items

//Declare spinlock here as global and initialize later.
spinlock_t lock;

int items = 0;

//Not sure why, but it seems to produce faster than consume.
void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    spinlock_lock(&lock);
    while(items >= MAX_ITEMS) {
      //Without this unlock then relock, the program will deadlock.
      spinlock_unlock(&lock);
      spinlock_lock(&lock);
      producer_wait_count++;
    }
    items++;
    histogram[items]++;
    spinlock_unlock(&lock);
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    spinlock_lock(&lock);
    while(items <= 0) {
      spinlock_unlock(&lock);
      spinlock_lock(&lock);
      consumer_wait_count++;
    }
    items--;
    histogram[items]++;
    spinlock_unlock(&lock);
  }
  return NULL;
}

int main (int argc, char** argv) {
  //Initialize the spinlock
  spinlock_create(&lock);
  uthread_t t[4];

  uthread_init (4);
  
  // TODO: Create Threads and Join
  //First 2 are producers, second 2 are consumers
  //This is not a super readable approach but it works for now.
  for(int i = 0; i < 2; i++) {
    t[i] = uthread_create(producer, NULL);
    t[i+2] = uthread_create(consumer, NULL);
  }

  //Join/Destroy threads after completion
  for(int i = 0; i < 4; i++) {
    uthread_join(t[i], NULL);
  }
  
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  assert (sum == sizeof (t) / sizeof (uthread_t) * NUM_ITERATIONS);
}
