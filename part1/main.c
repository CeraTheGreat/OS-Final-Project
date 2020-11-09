#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_TRAINERS 5
#define MAX_WAIT_SLOTS 6
#define MIN_TRAINERS 3
#define MIN_WAIT_SLOTS 3

struct _queue {
	size_t actual_wait_slots;
	uint16_t slots[MAX_WAIT_SLOTS + 1];
	size_t ins_pos;
	size_t read_pos;
} wait_queue;

uint16_t push_queue(uint16_t value);
uint16_t pop_queue();
bool queue_has_slot();
bool queue_has_value();

int training_floor[MAX_TRAINERS]; //training floor
int training = 0;	//keeps track of amt of trainers/customers on floor
int done_training = 0; //dno what this will do

//semaphores
sem_t empty; //number of empty slots
sem_t full; //number of slots filled
pthread_mutex_t mutex;

//Customers come in and look at the training floor.
//If there's an open spot on the floor, then the customer goes to the floor.
//If there's not an open spot, then the customer looks at the waiting room.
//If there's an open spot in the waiting room, then the customer goes to the waiting room.
//If there's not an open spot, then the customer leaves.

void *customer(void *cno){	
	pthread_mutex_lock(&mutex);
	if((training + 1) % (MAX_TRAINERS + 1) == 0) {	//if there's not room on the floor
		if (!queue_has_slot()) {	//if there's no space in waiting room
			printf("Customer %d left due to no space!\n",*((int *)cno));
			pthread_mutex_unlock(&mutex);	//unlock before exiting thread
			pthread_exit(cno);	//if no space in waiting room, leave (exit current thread)
		}
		else{	//There is space in waiting room
			push_queue(*((uint16_t *)cno)); 	//put customer in queue (waiting room)
			printf("Customer %d is in the wait queue!\n", *((int *)cno));
		}
	}
	else {	//space available on training floor
		training_floor[training] = *(int *)cno;	//Put customer onto workout floor
		printf("Customer %d going to training floor!\n", *((int *)cno));
		training = (training + 1)%(MAX_TRAINERS + 1);	//increase number of current customers on floor
	}
	pthread_mutex_unlock(&mutex);
	sem_post(&full);	//add space to full semaphore
}

//If trainers are waiting, grab the next customer that comes in till all customers are training.
//If a trainer is done training, they need to look at the waiting room.
//If a customer is in the waiting room, grab the first customer that entered.
//If a customer is not in the waiting room, busy wait till next customer comes.

void *trainer(void *tno) {
	sem_wait(&full);	//wait when no customers
	pthread_mutex_lock(&mutex);
	int customer_training = pop_queue();	//take customer off waitlist and start training IF COMING FROM TRAINING OTHER CUSTOMER
	printf("Trainer %d is training cutomer %d\n", *((int *)tno), customer_training);
	//do some stuff with customer and weights (part 2?)
	
	pthread_mutex_unlock(&mutex);
	sem_post(&empty);	//add space to empty semaphore
}

int main(int argc, char **argv) {
	/* set wait slot value, IDK where we get this */
	wait_queue.actual_wait_slots = 6;
	
	pthread_t train[MAX_TRAINERS], cust[20];	//delcare the trainer and customer threads. Amt of customer threads should be amount of customers coming?
	pthread_mutex_init(&mutex, NULL);			//initilize mutex
	sem_init(&empty,0,wait_queue.actual_wait_slots);	//initilize empty semaphore
	sem_init(&full,0,0);					//initilize full semaphore
	
	int a[] = {1,2,3,4,5}; //keeping track of trainers
	int b[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}; //keeping track of customers

	/* add
	printf("%d\n", push_queue(1));
	printf("%d\n", push_queue(2));
	printf("%d\n", push_queue(3));
	printf("%d\n", push_queue(4));
	printf("%d\n", push_queue(5));
	printf("%d\n", push_queue(6));
	printf("%d\n", push_queue(6));


	printf("%d\n", pop_queue());
	printf("%d\n", pop_queue());
	printf("%d\n", pop_queue());
	printf("%d\n", pop_queue());
	printf("%d\n", pop_queue());
	printf("%d\n", pop_queue());
	printf("%d\n", pop_queue());
	*/
	
	//todo:while queue is empty create customers at random time
	//todo:if no trainer available, "walk" to waiting room at certain speed?
	
	//Creates 6 threads(customers) that enter waiting room
	for(int i = 0; i < 20; i++) {
		pthread_create(&cust[i], NULL, (void *)customer, (void *)&b[i]);
	}
	
	//todo:while customer in queue and trainer available, start training
	//todo:after training, check queue once at certain speed?
	
	//Creates 5 trainers to train customers
//	for(int i = 0; i < MAX_TRAINERS; i++) {
//		pthread_create(&train[i], NULL, (void *)trainer, (void *)&a[i]);
//	}
	
	//Don't let main exit before threads do their thing
	for(int i = 0; i < 20; i++) {
		pthread_join(cust[i], NULL);
	}
	/*
	for(int i = 0; i < 5; i++) {
		pthread_join(train[i], NULL);
	}
	*/
	
	//destory mutex and semaphores before exiting
	pthread_mutex_destroy(&mutex);
	sem_destroy(&empty);
	sem_destroy(&full);
	
	return 0;
}

uint16_t push_queue(uint16_t value) {
	if (queue_has_slot()) {
		wait_queue.slots[wait_queue.ins_pos++] = value;
		wait_queue.ins_pos %= (wait_queue.actual_wait_slots + 1);
		return value;
	} else {
		/* return 0 on failure */
		return 0;
	}
}

uint16_t pop_queue() {
	if (queue_has_value()) {
		uint16_t value = wait_queue.slots[wait_queue.read_pos++];
		wait_queue.read_pos %= (wait_queue.actual_wait_slots + 1);
		return value;
	} else {
		/* return 0 on failure */
		return 0;
	}
}

bool queue_has_slot() {
	return (!((wait_queue.ins_pos + 1) % (wait_queue.actual_wait_slots + 1) ==
			wait_queue.read_pos));
}

bool queue_has_value() {
	return (!(wait_queue.ins_pos == wait_queue.read_pos));
}
