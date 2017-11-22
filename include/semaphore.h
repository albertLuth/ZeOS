#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <list.h>

struct semaphore_struct
{
	int ID;
	int owner;
	unsigned int value;
	struct list_head blocked_processes;

};
#endif /* !SEMAPHORE_H */
