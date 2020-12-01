#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/* as specified by the assignment */
#define MAX_TRAINERS 5
#define MAX_WAIT_SLOTS 6
#define MIN_TRAINERS 3
#define MIN_WAIT_SLOTS 3

/* maximum amount of time between customer spawns */
#define MAX_USLEEP 30

/* circular queue, FIFO */
struct _queue {
	size_t actual_wait_slots;
	uint16_t slots[MAX_WAIT_SLOTS + 1];
	size_t ins_pos;
	size_t read_pos;
};

uint16_t push_queue(uint16_t value);
uint16_t pop_queue();
bool queue_has_slot();
bool queue_has_value();

//void *customer_tf(void *argp);
//void *trainer_tf(void *argp);

void trainer(int num_customers);
void customer(int num_customers);
