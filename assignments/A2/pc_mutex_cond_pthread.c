#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define MAX_ITEMS 10
const int NUM_ITERATIONS = 200;
const int NUM_CONSUMERS  = 2;
const int NUM_PRODUCERS  = 2;

//Create both the mutex and condition
pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  consumer_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t  producer_condition = PTHREAD_COND_INITIALIZER;

int producer_wait_count;     // # of times producer had to wait
int consumer_wait_count;     // # of times consumer had to wait
int histogram [MAX_ITEMS+1]; // histogram [i] == # of times list stored i items
// MAKE 2 CONDITION VARIABLES TO LET THE THREADS KNOW IF THE CAN PRODUCE OR CONSUME

int items = 0;

void* producer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    pthread_mutex_lock(&mutex);
    while(items >= MAX_ITEMS) {
      pthread_cond_wait(&producer_condition, &mutex);
      producer_wait_count++;
    }
    items++;
    histogram[items]++;
    pthread_cond_signal(&consumer_condition);
    pthread_mutex_unlock(&mutex);
  }
  return NULL;
}

void* consumer (void* v) {
  for (int i=0; i<NUM_ITERATIONS; i++) {
    pthread_mutex_lock(&mutex);
    while(items <= 0) {
      pthread_cond_wait(&consumer_condition, &mutex);
      consumer_wait_count++;
    }
    items--;
    histogram[items]++;
    pthread_cond_signal(&producer_condition);
    pthread_mutex_unlock(&mutex);
    // TODO
  }
  return NULL;
}

int main (int argc, char** argv) {
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
  
  printf ("producer_wait_count=%d\nconsumer_wait_count=%d\n", producer_wait_count, consumer_wait_count);
  printf ("items value histogram:\n");
  int sum=0;
  for (int i = 0; i <= MAX_ITEMS; i++) {
    printf ("  items=%d, %d times\n", i, histogram [i]);
    sum += histogram [i];
  }
  printf ("  sum=%d\n", sum);
}
