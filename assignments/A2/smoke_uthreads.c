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
#include "uthread.h"
#include "uthread_mutex_cond.h"

#define NUM_ITERATIONS 1000
#define MAX_ITEMS 10
#define true 1
#define false 0

//Create each of the resource combination conditions
uthread_cond_t  tobacco_smoker_cond;
uthread_cond_t  match_smoker_cond;
uthread_cond_t  paper_smoker_cond;

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
  uthread_mutex_t mutex;
  uthread_cond_t match;
  uthread_cond_t paper;
  uthread_cond_t tobacco;
  uthread_cond_t smoke;
};

struct Agent* createAgent() {
  struct Agent* agent = malloc (sizeof (struct Agent));
  agent->mutex   = uthread_mutex_create();
  agent->paper   = uthread_cond_create (agent->mutex);
  agent->match   = uthread_cond_create (agent->mutex);
  agent->tobacco = uthread_cond_create (agent->mutex);
  agent->smoke   = uthread_cond_create (agent->mutex);
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
  
  uthread_mutex_lock (a->mutex);
    for (int i = 0; i < NUM_ITERATIONS; i++) {
      int r = random() % 3;
      signal_count [matching_smoker [r]] ++;
      int c = choices [r];
      if (c & MATCH) {
        VERBOSE_PRINT ("match available\n");
        uthread_cond_signal (a->match);
      }
      if (c & PAPER) {
        VERBOSE_PRINT ("paper available\n");
        uthread_cond_signal (a->paper);
      }
      if (c & TOBACCO) {
        VERBOSE_PRINT ("tobacco available\n");
        uthread_cond_signal (a->tobacco);
      }
      VERBOSE_PRINT ("agent is waiting for smoker to smoke\n");
      uthread_cond_wait (a->smoke);
    }
  uthread_mutex_unlock (a->mutex);
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
		uthread_cond_signal(paper_smoker_cond);
	}
	else if(tobacco_boolean && paper_boolean) {
		uthread_cond_signal(match_smoker_cond);
	}
	else if(match_boolean && paper_boolean){
		uthread_cond_signal(tobacco_smoker_cond);
	}else {
		//Don't do anything here
	}
	return NULL;
}

// ++++++++++++++++++ END SMOKER CHECKER CODE ++++++++++++++++++++

// ++++++++++++++++++ SMOKERS +++++++++++++++++++
void* tobacco_smoker (void* v) {
	struct Agent* agent = v;
	uthread_mutex_lock(agent->mutex);
	//We want to continue smokeing until the agent no longer offers ingredients.
	while(true) {
		uthread_cond_wait(tobacco_smoker_cond);
		smoke_count[4]++;

		paper_boolean = false;
		match_boolean = false;

		uthread_cond_signal(agent->smoke);
	}
	uthread_mutex_unlock(agent->mutex);
	return NULL;
}

void* match_smoker (void* v) {
	struct Agent* agent = v;
  uthread_mutex_lock(agent->mutex);
  while(true) {
    uthread_cond_wait(match_smoker_cond);
    smoke_count[1]++;

    paper_boolean = false;
		tobacco_boolean = false;

    uthread_cond_signal(agent->smoke);
  }
  uthread_mutex_unlock(agent->mutex);
  return NULL;
}

void* paper_smoker (void* v) {
	struct Agent* agent = v;
	uthread_mutex_lock(agent->mutex);
	while(true) {
    uthread_cond_wait(paper_smoker_cond);
    smoke_count[2]++;

		tobacco_boolean = false;
		match_boolean = false;

    uthread_cond_signal(agent->smoke);
  }
  uthread_mutex_unlock(agent->mutex);
  return NULL;
}

void* tobacco_available (void* v) {
	//This casts the variable back to an agent.
	struct Agent* agent = v;
  uthread_mutex_lock(agent->mutex);
  //We want to continue smokeing until the agent no longer offers ingredients.
  while(true) {
    //Lots of problems with deadlock here and I'm having problems with it
    uthread_cond_wait(agent->tobacco);
    tobacco_boolean = 1;
    find_correct_smoker();
  }
  uthread_mutex_unlock(agent->mutex);
  return NULL;
}

void* match_available (void* v) {
	struct Agent* agent = v;
  uthread_mutex_lock(agent->mutex);
  while(true) {
    uthread_cond_wait(agent->match);
    match_boolean = 1;
    find_correct_smoker();
  }
  uthread_mutex_unlock(agent->mutex);
  return NULL;
}

void* paper_available (void* v) {
	struct Agent* agent = v;
	uthread_mutex_lock(agent->mutex);
	while(true) {
    uthread_cond_wait(agent->paper);
    paper_boolean = 1;
    find_correct_smoker();
  }
  uthread_mutex_unlock(agent->mutex);
  return NULL;
}
// ++++++++++++++++++ END SMOKERS +++++++++++++++++++

//This main is SUPER SUPER UGLY but I could not figure a way to loop it
void* run() {
  //Init is required here
  uthread_init(7);
  struct Agent* created_agent = createAgent();

  //My created condition variables
  tobacco_smoker_cond = uthread_cond_create(created_agent->mutex);
  match_smoker_cond = uthread_cond_create(created_agent->mutex);
  paper_smoker_cond = uthread_cond_create(created_agent->mutex);

  //Create the agent and all of the smokers.
  //We need to pass the agent to each of the smokers so that
    //they can use the agents cond variables.

  uthread_t tobacco_available_var = uthread_create(tobacco_available, created_agent);
  uthread_t match_available_var = uthread_create(match_available, created_agent);
  uthread_t paper_available_var = uthread_create(paper_available, created_agent);

  uthread_t tobacco_smoker_var = uthread_create(tobacco_smoker, created_agent);
  uthread_t match_smoker_var = uthread_create(match_smoker, created_agent);
  uthread_t paper_smoker_var = uthread_create(paper_smoker, created_agent);

  uthread_t agent_var = uthread_create(agent, created_agent);

  //Destroy/join the threads once they are done running the method
  uthread_join(agent_var, NULL);

  return NULL;
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
