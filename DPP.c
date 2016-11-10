#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
	int position;
	int count;
	sem_t *forks;
	sem_t *lock;
} params_t;


void think(int position)
{
	printf("Philosopher %d thinking...\n", position);
}

void eat(int position)
{
	printf("Philosopher %d eating...\n", position);
}

void *philosopher(void *params)
{
	int i;
	params_t self = *(params_t *)params;

	for(i = 0; i < 4; i++) {
		think(self.position);

		sem_wait(self.lock);
		sem_wait(&self.forks[self.position]);
		sem_wait(&self.forks[(self.position + 1) % self.count]);
		eat(self.position);
		sem_post(&self.forks[self.position]);
		sem_post(&self.forks[(self.position + 1) % self.count]);
		sem_post(self.lock);
	}

	think(self.position);
	pthread_exit(NULL);
}

int main( int argc, char *argv[] )  {
	if( argc == 4 ){
		int N = atoi(argv[1]);
		int S = atoi(argv[2]);
		int T = atoi(argv[3]);
		printf("N: %d, S: %d, T: %d\n\n", N, S, T);
		
		sem_t lock;
		sem_t forks[N];
		pthread_t philosophers[N];
		pthread_t handler;
		
		
		/* Init all semaphores == N forks and 1 for crit sec */
		int i;
		for(i = 0; i < N; i++) {
			sem_init(&forks[i], 0, 1);
		}
		sem_init(&lock, 0, N - 1);
		
		/*Create and run all threads*/
		for(i = 0; i < N; i++) {
			params_t *arg = malloc(sizeof(params_t));
			arg->position = i;
			arg->count = N;
			arg->lock = &lock;
			arg->forks = forks;

			pthread_create(&philosophers[i], NULL, philosopher, (void *)arg);
		}
		
	}else{
		printf("Please pass 3 arguments.\n");
	}
}