#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <sys/prctl.h>
#include <signal.h>

#include "main.h"

struct trainer_s trainer;

int num_trainers = MAX_TRAINERS;
int num_customers;
sem_t cus;
sem_t train;
sem_t mutex;

//void trainer(int num_customers);
//void customer(int num_customers);

int main(int argc, char **argv) {
	/* set up shared memory for processes */
	key_t sharedm_key = ftok(PROJECT_PATH, PROJECT_ID);
	int sharedm_id = shmget(
			sharedm_key,
			sizeof(struct sharedm_s),
			0666 | IPC_CREAT);

	/* attatch sharedmem to this process so we can set things up */
	struct sharedm_s* main_share = (struct sharedm_s*) shmat(sharedm_id, (void*)0, 0);

	/* initialize mutex in shared memory */
	//pthread_mutex_init(&main_share->checking_mutex, NULL);
	sem_init(&main_share->semaphore, 0, SEMAPHORE_INIT);

	/* set wait slot value in shared memory, IDK where we get this */
	main_share->wait_queue.actual_wait_slots = 6;

	/* create trainer processes, assigned id by i */
	for (int i = 0; i < 20; i++) {
		pid_t child_pid;

		if ((child_pid = fork()) == 0) {
			/* now we're in a trainer process - let's set things up */
			/* kill child when parent dies */
			prctl(PR_SET_PDEATHSIG, SIGHUP);

			trainer.id = i;

			/* get the shared mem info */
			key_t key = ftok(PROJECT_PATH, PROJECT_ID);
			int shmid = shmget( 
					key, 
					sizeof(struct sharedm_s), 
					0666 | IPC_CREAT);

			/* attatch to shared memory */
			trainer.share = (struct sharedm_s*) shmat(shmid, (void*)0, 0);

			/* create pipes for trainer communication */
			pipe(trainer.share->pipes[trainer.id].in);
			pipe(trainer.share->pipes[trainer.id].out);

			run_trainer();
		}
	}

	// destroy shared memory stuff when we're done
	shmctl(sharedm_id, IPC_RMID, NULL);
	//pthread_mutex_destroy(&main_share->checking_mutex);

	////Need to do some testing. Print some stuff knowing if everything is working properly and to show.
	//for (int i = 0; i < 20; i++) { //When does this become false? Idk when program should end.
	//	num_customers = 0;
	//	customer(num_customers);
	//	int pid = fork();	// check for error
	//	trainer(num_customers);
	//}

	return 0;
}

uint16_t push_queue(uint16_t value, struct queue_s *queue) {
	if (queue_has_slot(queue)) {
		queue->slots[queue->ins_pos++] = value;
		queue->ins_pos %= (queue->actual_wait_slots + 1);
		return value;
	} else {
		/* return 0 on failure */
		return 0;
	}
}

uint16_t pop_queue(struct queue_s *queue) {
	if (queue_has_value(queue)) {
		uint16_t value = queue->slots[queue->read_pos++];
		queue->read_pos %= (queue->actual_wait_slots + 1);
		return value;
	} else {
		/* return 0 on failure */
		return 0;
	}
}

bool queue_has_slot(struct queue_s *queue) {
	return (!((queue->ins_pos + 1) % (queue->actual_wait_slots + 1) ==
			queue->read_pos));
}

bool queue_has_value(struct queue_s *queue) {
	return (!(queue->ins_pos == queue->read_pos));
}

void run_trainer() {
	char buff[PIPE_BUFF_SIZE];
	ssize_t chars_read;
	while (true) {
		/* set trainer as idle */
		trainer.share->is_trainer_idle[trainer.id] = true;
		/* blocking read, wait for customer*/
		chars_read = read(trainer.share->pipes[trainer.id].in[READ], buff, PIPE_BUFF_SIZE);

		train_customer();

		/* when training is complete, check for customers in the waiting room */
		/* lock semaphore to exclude other searchers */
		//sem_wait(&trainer.share->semaphore);
		if (queue_has_value(&trainer.share->wait_queue)) {
			/* TODO: pass trainer id to customer and they will join trainer */
		}

		/* release semaphore after we've done our search */
		//sem_post(&trainer.share->semaphore);
	}
}

void train_customer() {
	/* read list, instruct user on what to do, wait for them to finish */
	/* sleep for now, until it's implimented */
	sleep(2);
}

//void trainer(int num_customers) {
//	sem_wait(&cus);
//	sem_wait(&mutex);
//	num_customers--;
//	if (num_trainers > 0) { // if num_trainers > 0 decrease amt of available trainers and set trainer available
//		num_trainers--;
//		sem_post(&train);
//	}
//	sem_post(&mutex);
//	//do_coaching();
//}
//
//void customer(int num_customers) {
//	sem_wait(&mutex);
//	if(queue_has_slot()) {
//		num_customers++;
//		sem_post(&cus);
//		sem_post(&mutex);
//		sem_wait(&train);
//		if (num_trainers < 6) {
//			//do_training();
//			num_trainers++; //need another semaphore? to prevent context switching?
//		}
//	}
//	else {
//		sem_post(&mutex);
//	}
//}
