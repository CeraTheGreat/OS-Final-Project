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
#include <stdlib.h>
#include <fcntl.h>

#include "main.h"

struct context_s trainer;
struct context_s customer;

int record_book_fd;

int main(int argc, char **argv) {

	/* open record book & overwrite */
	record_book_fd = open(RECORD_BOOK_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	/* set up shared memory for processes */
	key_t sharedm_key = ftok(PROJECT_PATH, PROJECT_ID);
	int sharedm_id = shmget(
			sharedm_key,
			sizeof(struct sharedm_s),
			0666 | IPC_CREAT);

	/* attatch sharedmem to this process so we can set things up */
	struct sharedm_s* main_share = (struct sharedm_s*) shmat(sharedm_id, (void*)0, 0);

	/* set up pipes for waiting room */
	for (int i = 0; i < MAX_WAIT_SLOTS + 1; i++) {
		pipe(main_share->wait_queue.slots[i].fd);
	}

	for (int i = 0; i < MAX_TRAINERS; i++) {
		/* create pipes for trainer communication */
		pipe(main_share->pipes[i].in);
		pipe(main_share->pipes[i].out);
	}

	/* initialize mutex in shared memory */
	sem_init(&main_share->mutex, 0, SEMAPHORE_INIT);
	sem_init(&main_share->semaphore, 0, SEMAPHORE_INIT);

	/* set wait slot value in shared memory, IDK where we get this */
	main_share->wait_queue.actual_wait_slots = 6;

	/* create trainer processes, assigned id by i */
	for (int i = 0; i < MAX_TRAINERS; i++) {
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


			/* set trainer as idle to start */
			trainer.share->is_trainer_idle[trainer.id] = true;

			run_trainer();

			/* end process here */
			exit(0);
		}
	}

	sleep(4);

	/* spawn customers at random intervals */
	for (int i = 0; i < 20; i++) {
		pid_t child_pid;

		if ((child_pid = fork()) == 0) {
			/* now in a customer process - let's set things up */
			/* kill child when parent dies */
			prctl(PR_SET_PDEATHSIG, SIGHUP);

			customer.id = i;

			/* get the shared mem info */
			key_t key = ftok(PROJECT_PATH, PROJECT_ID);
			int shmid = shmget( 
					key, 
					sizeof(struct sharedm_s), 
					0666 | IPC_CREAT);

			/* attatch to shared memory */
			customer.share = (struct sharedm_s*) shmat(shmid, (void*)0, 0);

			run_customer();

			/* end process here */
			exit(0);

		} else {
			/* wait to start a new customer */
			sleep(1);
		}
	}

	/* once all customers have been created, wait untill trainers are all done */
	bool still_training = true;
	while (still_training) {
		still_training = false;
		for (int i = 0; i < MAX_TRAINERS; i++) {
			if (!main_share->is_trainer_idle[i]) {
				still_training = true;
				break;
			} 
		}
		/* sleep while we poll to keep the CPU happy */
		sleep(2);
	}

	/* close pipes */
	for (int i = 0; i < MAX_WAIT_SLOTS + 1; i++) {
		close(main_share->wait_queue.slots[i].fd[READ]);
		close(main_share->wait_queue.slots[i].fd[WRITE]);
	}

	// destroy shared memory stuff when we're done
	shmctl(sharedm_id, IPC_RMID, NULL);
	
	//pthread_mutex_destroy(&main_share->checking_mutex);

	return 0;
}

int push_queue(struct queue_s *queue) {
	if (queue_has_slot(queue)) {
		/* get read end of pipe */
		int fd = queue->slots[queue->ins_pos++].fd[READ];
		queue->ins_pos %= (queue->actual_wait_slots + 1);
		return fd;
	} else {
		/* return 0 on failure */
		return 0;
	}
}

int pop_queue(struct queue_s *queue) {
	if (queue_has_value(queue)) {
		int fd = queue->slots[queue->read_pos++].fd[WRITE];
		queue->read_pos %= (queue->actual_wait_slots + 1);
		return fd;
	} else {
		/* return 0 on failure */
		return 0;
	}
}

bool queue_has_slot(struct queue_s *queue) {
	return (!((queue->ins_pos + 1) % (queue->actual_wait_slots + 1) == queue->read_pos));
}

bool queue_has_value(struct queue_s *queue) {
	return (!(queue->ins_pos == queue->read_pos));
}

void run_trainer() {
	char buff[PIPE_BUFF_SIZE];
	buff[0] = 'a';
	ssize_t chars_read;

	while (true) {

		printf("trainer %d waiting on training floor...\n", trainer.id);

		/* blocking read, wait for customer to be ready*/
		chars_read = read(trainer.share->pipes[trainer.id].in[READ], buff, PIPE_BUFF_SIZE);

		train_customer();

		/* when training is complete, check for customers in the waiting room */
		/* lock semaphore to exclude other searchers */
		sem_wait(&trainer.share->semaphore);
		if (queue_has_value(&trainer.share->wait_queue)) {

			printf("trainer %d waking customer\n", trainer.id);

			/* TODO: pass trainer id to customer and they will join trainer */
			int customer_fd = pop_queue(&trainer.share->wait_queue);
			write(customer_fd, (char*) &trainer.id, 1);

			/* release semaphore after we've done critical operations */
			sem_post(&trainer.share->semaphore);

		} else {
			/* set trainer as idle */
			trainer.share->is_trainer_idle[trainer.id] = true;

			/* release semaphore after we've done critical operations */
			sem_post(&trainer.share->semaphore);
		}
	}
}

void train_customer() {
	/* read list, instruct user on what to do, wait for them to finish */
	/* sleep for now, until it's implimented */
	sleep(10);
}

void run_customer() {
	/* upon starting, take semaphore and check for availability */
	sem_wait(&customer.share->semaphore);
	
	/* check workroom for idle trainers first */
	int trainer_id = 0;

	for (; trainer_id < MAX_TRAINERS; trainer_id++) {
		if (customer.share->is_trainer_idle[trainer_id]) {
			break;
		}
	}

	/* we found a free trainer - start training with them */
	if (trainer_id < MAX_TRAINERS) {
		customer.share->is_trainer_idle[trainer_id] = false;
		/* wake trainer */

		printf("customer %d waking trainer %d\n", customer.id, trainer_id);

		write(customer.share->pipes[trainer_id].in[WRITE], "\1", 2);
		/* release semaphore */
		sem_post(&customer.share->semaphore);
		recieve_training(trainer_id);
		return;	
	
	/* we didn't find a free trainer - check waiting room*/
	} else if (queue_has_slot(&customer.share->wait_queue)) {
		int wait_fd = push_queue(&customer.share->wait_queue);
		sem_post(&customer.share->semaphore);
	
		printf("customer %d waiting in queue\n", customer.id);

		/* wait for trainer id through pipe */
		char buff[PIPE_BUFF_SIZE];
		ssize_t chars_read;

		chars_read = read(wait_fd, buff, PIPE_BUFF_SIZE);

		/* when a trainer invites a customer, customer announces when ready */
		trainer_id = (int) buff[0];

		printf("customer %d woken by trainer %d\n", customer.id, trainer_id);

		write(customer.share->pipes[trainer_id].in[WRITE], "\1", 2);
		recieve_training(trainer_id);
			
		return;
	
	/* no waitroom space, just leave */
	} else {
		printf("customer %d left with no space\n", customer.id);
		sem_post(&customer.share->semaphore);
		return;
	}
}

void recieve_training(int trainer_id) {
	/* read list, instruct user on what to do, wait for them to finish */
	/* sleep for now, until it's implimented */
	sleep(2);
}
