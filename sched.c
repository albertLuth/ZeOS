/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

/**
 * Container for the Task array and 2 additional pages (the first and the last one)
 * to protect against out of bound accesses.
 */
union task_union protected_tasks[NR_TASKS+2]
  __attribute__((__section__(".data.task")));

union task_union *task = &protected_tasks[1]; /* == union task_union task[NR_TASKS] */

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;


/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
	return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
	return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}


void allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 
}

void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");

	while(1)
	{
	
	}
}

void init_readyqueue()
{
	INIT_LIST_HEAD(&readyqueue);
}

void init_freequeue()
{
	INIT_LIST_HEAD(&freequeue);
	int i;
	for(i = 0; i < NR_TASKS; i++) {
		task[i].task.PID = -1;
		list_add(&(task[i].task.list),&freequeue);
	}

}

void init_idle (void)
{ 
	
	struct list_head *first =  list_first(&freequeue);		//agafar el primer element de la frequeue
	list_del(first);										//el proces ja no esta en la frequeue
	
	struct task_struct pcb = *list_head_to_task_struct(first);

	pcb.PID = 0;

	allocate_DIR(&pcb);		//inicialitza el camp dir_pages_baseAddr per guardar l'espai d'adreces

	union task_union task_u = (union task_union)pcb;

	task_u.stack[KERNEL_STACK_SIZE-1] = &(cpu_idle);		//adreça de retorn 
	task_u.stack[KERNEL_STACK_SIZE-2] = 0;					//ebp
	task_u.task.kernel_esp = &(task_u.stack[KERNEL_STACK_SIZE-2]);
	//task_u.task.kernel_esp = KERNEL_ESP(&task_u);			//esp

	idle_task = &pcb;
}

void init_task1(void)
{
	
	struct list_head *first =  list_first(&freequeue);		//agafar el primer element de la frequeue
	list_del(first);										//el proces ja no esta en la frequeue
	
	struct task_struct pcb = *list_head_to_task_struct(first);

	pcb.PID = 1;

	allocate_DIR(&pcb);		//inicialitza el camp dir_pages_baseAddr per guardar l'espai d'adreces

	set_user_pages(&pcb);

	union task_union task_u = (union task_union)pcb;

	tss.esp0 = KERNEL_ESP(&task_u);

	set_cr3(pcb.dir_pages_baseAddr);

}


void init_sched()
{

	init_freequeue();
	init_readyqueue();
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

