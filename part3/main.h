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

/* record book name */
#define RECORD_BOOK_NAME "records.txt"

/* shared memory constants */
#define PROJECT_PATH ".share"
#define PROJECT_ID 1

#define SEMAPHORE_INIT 1

#define PIPE_BUFF_SIZE 128
#define READ 0
#define WRITE 1



/* for communication between trainer and customer */
struct trainer_pipes {
	int in[2];
	int out[2];
};

struct waitroom_pipes {
	int fd[2];
};

/* circular queue, FIFO */
struct queue_s {
	size_t actual_wait_slots;
	struct waitroom_pipes slots[MAX_WAIT_SLOTS + 1];
	size_t ins_pos;
	size_t read_pos;
};


/* shared memory for ipc */
struct sharedm_s {
	sem_t mutex;
	sem_t semaphore;
	struct queue_s wait_queue;
	struct trainer_pipes pipes[MAX_TRAINERS];
	bool is_trainer_idle[MAX_TRAINERS];
};

/* everything a single trainer needs to know */
struct context_s{
	struct sharedm_s *share;
	uint8_t id;
};

int push_queue(struct queue_s *queue);
int pop_queue(struct queue_s *queue);

bool queue_has_slot(struct queue_s *queue);
bool queue_has_value(struct queue_s *queue);

void run_trainer();
void train_customer(int customer_id);

void run_customer();
void recieve_training(int trainer_id);

