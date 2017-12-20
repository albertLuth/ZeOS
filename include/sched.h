/*
 * sched.h - Estructures i macros pel tractament de processos
 */
//((0.5*((0.5*4.7) + (0.5*2.5) )) + (0.5*((0.35*6.5) + (0.35*4.25) + (0.30*7.5))) + 0.7)*(10/11)
#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>
#include <stats.h>
#include <semaphore.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024
#define SEMAPHORES_SIZE 20

enum state_t { ST_RUN, ST_READY, ST_BLOCKED };

struct task_struct {	//PCB
	int PID;			/* Process ID. This MUST be the first field of the struct. */
	int kernel_esp;
	enum state_t state;
	page_table_entry * dir_pages_baseAddr;
	struct list_head list;
	struct stats statistics;
	int quantum_rr;
	int dir_pos;
	void * program_break;
};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

extern int bytesCircularBufferOcupados;
extern int posicionInicialParaLeer;


extern union task_union protected_tasks[NR_TASKS+2];
struct semaphore_struct	semaphores[SEMAPHORES_SIZE];
extern union task_union *task; /* Vector de tasques */
struct task_struct *idle_task;

struct list_head freequeue;
struct list_head readyqueue;

struct list_head keyboardqueue;

extern char circularbuffer[512];

#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

/* Inicialitza les dades del proces inicial */
void init_readyqueue();

void init_freequeue();

void init_semaphores();

void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void task_switch(union task_union*t);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int get_pos_DIR(struct task_struct *t);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();
void schedule();
int get_quantum(struct task_struct * t);
void set_quantum(struct task_struct * t, int new_quantum);

#endif  /* __SCHED_H__ */
