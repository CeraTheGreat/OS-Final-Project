#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

/* as specified by the assignment */
#define MAX_TRAINERS 5
#define MAX_WAIT_SLOTS 6
#define MIN_TRAINERS 3
#define MIN_WAIT_SLOTS 3

/* maximum amount of time between customer spawns */
#define MAX_USLEEP 30

/* shared memory constants */
#define PROJECT_PATH "share"
#define PROJECT_ID 1

#define SEMAPHORE_INIT 1

#define PIPE_BUFF_SIZE 128
#define WRITE 0
#define READ 1

/* circular queue, FIFO */
struct queue_s {
	size_t actual_wait_slots;
	uint16_t slots[MAX_WAIT_SLOTS + 1];
	size_t ins_pos;
	size_t read_pos;
};

/* for communication between trainer and customer */
struct trainer_pipes {
	int in[2];
	int out[2];
};

struct waitroom_pipes {
	int fd[2];
};

/* shared memory for ipc */
struct sharedm_s {
	sem_t semaphore;
	//pthread_mutex_t checking_mutex;
	struct queue_s wait_queue;
	struct trainer_pipes pipes[MAX_TRAINERS];
	bool is_trainer_idle[MAX_TRAINERS];
};

/* everything a single trainer needs to know */
struct trainer_s {
	struct sharedm_s *share;
	uint8_t id;
};

uint16_t push_queue(uint16_t value, struct queue_s *queue);
uint16_t pop_queue(struct queue_s *queue);
bool queue_has_slot(struct queue_s *queue);
bool queue_has_value(struct queue_s *queue);

//void *customer_tf(void *argp);
//void *trainer_tf(void *argp);
void run_trainer();
void train_customer();

//void trainer(int num_customers);
//void customer(int num_customers);
