//The general idea for this is to have a condition variable
//for each of the three combinations that the agent can offer.
//The smoker that requires that resource combination will listen
//and smoke signaling the agent to offer two more resources.

//To do the above, we can use a set of booleans to decide when
//the correct set are true.
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000
#define MAX_ITEMS 10
#define true 1
#define false 0

//Create mutex and each of the resource combination conditions
pthread_cond_t  tobacco_smoker_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t  match_smoker_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t  paper_smoker_cond = PTHREAD_COND_INITIALIZER;

int tobacco_boolean = 0;
int match_boolean = 0;
int paper_boolean = 0;

// ++++++++++++++++++++++ AGENT CODE +++++++++++++++++++++++
#ifdef VERBOSE
#define VERBOSE_PRINT(S, ...) printf (S, ##__VA_ARGS__);
#else
#define VERBOSE_PRINT(S, ...) ;
#endif

struct Agent {
  pthread_mutex_t mutex;
  pthread_cond_t match;
  pthread_cond_t paper;
  pthread_cond_t tobacco;
  pthread_cond_t smoke;
};

struct Agent* createAgent() {
  	struct Agent* agent = malloc (sizeof (struct Agent));
  	//I really don't know why but I cant use
  	//PTHREAD_COND_INITIALIZER here. So we declare them
  	//in a different way.
  	pthread_mutex_init(&agent->mutex, NULL);
  	pthread_cond_init(&agent->paper, NULL);
  	pthread_cond_init(&agent->match, NULL);
  	pthread_cond_init(&agent->tobacco, NULL);
  	pthread_cond_init(&agent->smoke, NULL);
  	return agent;
}

//
// TODO
// You will probably need to add some procedures and struct etc.
//

/**
 * You might find these declarations helpful.
 *   Note that Resource enum had values 1, 2 and 4 so you can combine resources;
 *   e.g., having a MATCH and PAPER is the value MATCH | PAPER == 1 | 2 == 3
 */
enum Resource            {    MATCH = 1, PAPER = 2,   TOBACCO = 4};
char* resource_name [] = {"", "match",   "paper", "", "tobacco"};

int signal_count [5];  // # of times resource signalled
int smoke_count  [5];  // # of times smoker with resource smoked

/**
 * This is the agent procedure.  It is complete and you shouldn't change it in
 * any material way.  You can re-write it if you like, but be sure that all it does
 * is choose 2 random reasources, signal their condition variables, and then wait
 * wait for a smoker to smoke.
 */
void* agent (void* av) {
  struct Agent* a = av;
  static const int choices[]         = {MATCH|PAPER, MATCH|TOBACCO, PAPER|TOBACCO};
  static const int matching_smoker[] = {TOBACCO,     PAPER,         MATCH};

  pthread_mutex_lock (&a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      //Always seems to pick match and tobacco on the first round.
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        pthread_cond_signal (&a->match);
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        pthread_cond_signal (&a->paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        pthread_cond_signal (&a->tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      pthread_cond_wait (&a->smoke, &a->mutex);
    }
  pthread_mutex_unlock (&a->mutex);
  return NULL;
}
// ++++++++++++++++++ END AGENT CODE +++++++++++++++++++++

// ++++++++++++++++++ SMOKER CHECKER CODE ++++++++++++++++++++

void* find_correct_smoker() {
	//This meathod will always be called twice
		//once for each resourse.
		//Beacuse of this, we need a case for when only
		//one resourse boolean has been set.
		//This is covered by the else statement.
	if(tobacco_boolean && match_boolean) {
		pthread_cond_signal(&paper_smoker_cond);
	}
	else if(tobacco_boolean && paper_boolean) {
		pthread_cond_signal(&match_smoker_cond);
	}
	else if(match_boolean && paper_boolean){
		pthread_cond_signal(&tobacco_smoker_cond);
	}else {
		//Don't do anything here
	}
	return NULL;
}

// ++++++++++++++++++ END SMOKER CHECKER CODE ++++++++++++++++++++

// ++++++++++++++++++ SMOKERS +++++++++++++++++++
void* tobacco_smoker (void* v) {
	struct Agent* agent = v;
	pthread_mutex_lock(&agent->mutex);
	//We want to continue smokeing until the agent no longer offers ingredients.
	while(true) {
		pthread_cond_wait(&tobacco_smoker_cond, &agent->mutex);
		smoke_count[4]++;

		paper_boolean = false;
		match_boolean = false;

		pthread_cond_signal(&agent->smoke);
	}
	pthread_mutex_unlock(&agent->mutex);
	return NULL;
}

void* match_smoker (void* v) {
	struct Agent* agent = v;
    pthread_mutex_lock(&agent->mutex);
    while(true) {
      	pthread_cond_wait(&match_smoker_cond, &agent->mutex);
      	smoke_count[1]++;

     	paper_boolean = false;
		tobacco_boolean = false;

      pthread_cond_signal(&agent->smoke);
    }
    pthread_mutex_unlock(&agent->mutex);
  return NULL;
}

void* paper_smoker (void* v) {
	struct Agent* agent = v;
	pthread_mutex_lock(&agent->mutex);
	while(true) {
      	pthread_cond_wait(&paper_smoker_cond, &agent->mutex);
      	smoke_count[2]++;

		tobacco_boolean = false;
		match_boolean = false;

      	pthread_cond_signal(&agent->smoke);
    }
    pthread_mutex_unlock(&agent->mutex);
  return NULL;
}

void* tobacco_available (void* v) {
	//This casts the variable back to an agent.
	struct Agent* agent = v;
    pthread_mutex_lock(&agent->mutex);
    //We want to continue smokeing until the agent no longer offers ingredients.
    while(true) {
    	//Lots of problems with deadlock here and I'm having problems with it
      	pthread_cond_wait(&agent->tobacco, &agent->mutex);
      	tobacco_boolean = true;
      	find_correct_smoker();
  	}
  	pthread_mutex_unlock(&agent->mutex);
  	return NULL;
}

void* match_available (void* v) {
	struct Agent* agent = v;
    pthread_mutex_lock(&agent->mutex);
    while(true) {
      pthread_cond_wait(&agent->match, &agent->mutex);
      match_boolean = true;
      find_correct_smoker();
    }
    pthread_mutex_unlock(&agent->mutex);
  	return NULL;
}

void* paper_available (void* v) {
	struct Agent* agent = v;
	pthread_mutex_lock(&agent->mutex);
	while(true) {
      pthread_cond_wait(&agent->paper, &agent->mutex);
      paper_boolean = true;
      find_correct_smoker();
    }
    pthread_mutex_unlock(&agent->mutex);
  	return NULL;
}
// ++++++++++++++++++ END SMOKERS +++++++++++++++++++

//This main is SUPER SUPER UGLY but I could not figure a way to loop it
void* run() {
	struct Agent* created_agent = createAgent();

  	pthread_t tobacco_available_var;
  	pthread_t match_available_var;
  	pthread_t paper_available_var;

  	pthread_t tobacco_smoker_var;
  	pthread_t match_smoker_var;
  	pthread_t paper_smoker_var;

  	pthread_t agent_var; 
  
  	//Create the agent and all of the smokers.
  	//We need to pass the agent to each of the smokers so that
  		//they can use the agents cond variables.

  	pthread_create(&tobacco_available_var, NULL, tobacco_available, created_agent);
  	pthread_create(&match_available_var, NULL, match_available, created_agent);
  	pthread_create(&paper_available_var, NULL, paper_available, created_agent);

  	pthread_create(&tobacco_smoker_var, NULL, tobacco_smoker, created_agent);
  	pthread_create(&match_smoker_var, NULL, match_smoker, created_agent);
  	pthread_create(&paper_smoker_var, NULL, paper_smoker, created_agent);

  	//THE AGENTS CREATION ORDER MATTERS!!!!!
  	//THIS WAS WHY IT WAS DAEDLOCKING!!!!
  	pthread_create(&agent_var, NULL, agent, created_agent);

  	//Destroy/join the threads once they are done running the method
  	pthread_join(agent_var, NULL);

  	//I don't need to rejoin the threads like I thought I did.
  	//pthread_join(tobacco_available_var, NULL);
  	//pthread_join(match_available_var, NULL);
  	//pthread_join(paper_available_var, NULL);

  	//pthread_join(tobacco_smoker_var, NULL);
  	//pthread_join(match_smoker_var, NULL);
  	//pthread_join(paper_smoker_var, NULL);
}

//IT KEEPS DEADLOCKING AND I DON'T KNOW WHY!!!!!!!!
int main (int argc, char** argv) {
	run();

  	assert (signal_count [MATCH]   == smoke_count [MATCH]);
 	assert (signal_count [PAPER]   == smoke_count [PAPER]);
  	assert (signal_count [TOBACCO] == smoke_count [TOBACCO]);
  	assert (smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO] == NUM_ITERATIONS);

  	printf ("Smoke counts: %d matches, %d paper, %d tobacco\n",
          smoke_count [MATCH], smoke_count [PAPER], smoke_count [TOBACCO]);
  	printf("Total times smoked: %d\n", smoke_count [MATCH] + smoke_count [PAPER] + smoke_count [TOBACCO]);
}
