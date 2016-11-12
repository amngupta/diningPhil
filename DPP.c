#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
/*
Student name and No.: Kanak Kabara - 3035614221
UID: 3035164221
Development Platform: Ubuntu 14.04
Last Modified Date: 13th November 2016
Compilation: gcc -pthread DPP.c -o DPP
*/

//Structure to pass info to a philosopher
typedef struct {
	int position; 						//Philosophers position around the table
	sem_t *forks; 						//Array of semaphores that act as the forks
	sem_t *lock;						//Shared semaphore lock to enter critical sections
} philStruct;

int cont = 1, cont2 = 1;					//cont = 1 == Philosophers keep executing, cont2 = 1 == Handler keep executing
char **stateArr;						//Array to store the state of each philosopher 
int *forksArr;							//Array to store fork info (who holds which fork)
int N = 0;							//Number of Philosophers

void think(int position)					//Philosopher 'position' starts thinking
{
	stateArr[position] = "Thinking";			//Set state to thinking
	usleep((rand()%10000000)+1);				//Think for random amount of time
	stateArr[position] = "Waiting";				//Set state to waiting
}

void eat(int position)						//Philosopher 'position' starts eating
{
	stateArr[position] = "Eating";				//Set state to eating
	usleep((rand()%10000000)+1);				//Eat for random amount of time
	stateArr[position] = "Thinking";			//Set state to thinking
}

void *philosopher(void *params)					//fucntion for each philosopher
{
	philStruct self = *(philStruct *)params;		//get each philosopher's identity
	int i, secondFork = ((self.position + 1) % N);		//Fork1 = position of phil, Fork2 = position + 1, but this will be >N for Nth philo, so modulo N to get right fork
	
	while(cont==1) {			
		think(self.position);				//start thinking on position...
		sem_wait(self.lock);				//after thinking, entering crit sec, so get sema
		
		sem_wait(&self.forks[self.position]);		//get left fork
		forksArr[self.position] = self.position;	//mark left fork as taken
		
		sem_wait(&self.forks[secondFork]);		//get right fork
		forksArr[secondFork] = self.position;		//mark right fork as taken
		
		eat(self.position);				//have both forks, so start eating on position...
		
		sem_post(&self.forks[self.position]);		//drop left fork
		forksArr[self.position] = -1;			//mark left fork as free
		
		sem_post(&self.forks[(self.position + 1) % N]);	//drop right fork
		forksArr[secondFork] = -1;			//mark right fork as free

		sem_post(self.lock);				//Leaving crit sec, so drop sema
	}
	
	stateArr[self.position] = "Terminated";			//Once exited while loop, thread is terminated. so mark state as terminated		
	pthread_exit(NULL);					//Exit the thread
}

void *handler(void *params)
{
	int i, term;						//counter for terminated threads
	unsigned int usecs = 500000;				//usecs = time between each probe by watcher thread
	while(cont2==1 && term!=N) {				//while all threads not terminated
		int avail=0, use=0, waiting=0, eating=0, th=0;
		term=0;
		printf("\e[1;1H\e[2J");				//Flush console window
		printf("Philo\t\tState\t\t\tFork\t\tHeld By\n");
		for(i = 0; i < N; i++) {
			//Find current state of the philo and update respective counter
			printf("[%d]:\t\t%-10s\t\t", i, stateArr[i]);
			if(strcmp("Terminated", stateArr[i])==0)
				term++;
			else if(strcmp("Eating", stateArr[i])==0)
				eating++;
			else if(strcmp("Waiting", stateArr[i])==0)
				waiting++;
			else if(strcmp("Thinking", stateArr[i])==0)
				th++;
			
								//If forks==-1, it means it is free
			if(forksArr[i]==-1){
				printf("[%d]:\t\t%-10s\n", i, "Free"); 
				avail++;
			}else{
			 	printf("[%d]:\t\t%d\n", i, forksArr[i]); 
			 	use++;	
			}
		}
		printf("Th=%d Wa=%d Ea=%d\t\t\t\tUse=%d Avail=%d\n", th, waiting, eating, use, avail);
		usleep(usecs);
	}
	
	pthread_exit(NULL);					//Once all philo threads have terminated, terminate watcher thread
}

int main( int argc, char *argv[] )  {
	if( argc == 4 ){
		//Parse string commands to int values
		N = atoi(argv[1]);
		int S = atoi(argv[2]);
		int T = atoi(argv[3]);
		
		srand(S);					//Seed for random
		
		sem_t lock; 					//mutex lock for crit sec
		sem_t forks[N];					//semaphores for each fork
		pthread_t philosophers[N];			//1 thread per philo
		pthread_t handlerT;				//handler/watcher thread
		
		// Init all semaphores
		int i;
		for(i = 0; i < N; i++) {
			sem_init(&forks[i], 0, 1); 		//Init to 1 so that only 1 philo can use each fork at a time
		}
		sem_init(&lock, 0, N - 1); 			//init lock to N-1 so that only N-1 philos can try and acquire a fork. This is done to avoid deadlocks. 
		
		//Init arrays to store info for watcher thread
		stateArr = (char **)malloc(N * sizeof(char *));
		forksArr = (int *)malloc(N * sizeof(int));
		for(i = 0; i < N; i++) {
			stateArr[i] = "Thinking"; 		//Mark initial state as thinking
			forksArr[i] = -1;			//Mark fork as free
		}

		//Create and run all threads
		for(i = 0; i < N; i++) {
			philStruct *arg = malloc(sizeof(philStruct));
			arg->position = i;			//identifier for each thread
			arg->lock = &lock;	
			arg->forks = forks;

			if(pthread_create(&philosophers[i], NULL, philosopher, (void *)arg) != 0)
				perror("Error creating PhiloThread: ");
		}
		if(pthread_create(&handlerT, NULL, handler, NULL) != 0)
			perror("Error creating WatcherThread: ");

		sleep(T);					//Main thread sleeps for T seconds before..
		cont = 0;					//.. instructing all philo threads to stop executing 
		
		if(pthread_join(handlerT, NULL)==0)		//Once handler/watcher thread exits..
			printf("\nHandler thread terminated\n");
			
		pthread_exit(NULL);
	}else{
		printf("Please pass 3 arguments, (Number of Philosophers, (Seed for rand()), (Time to run the simulation).\n"); 		//Error shown if all 3 arguments are not passed
	}
}