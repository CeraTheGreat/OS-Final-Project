#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

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

int main(int argc, char **argv) {
	/* set wait slot value, IDK where we get this */
	wait_queue.actual_wait_slots = 6;

	/* add */
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
