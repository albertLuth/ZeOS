/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

int bytesCircularBufferOcupados = 0;
int posicionInicialParaLeer = 0;

char circularbuffer[512];


int remaining_quantum = 0;
#define DEFAULT_QUANTUM 10

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

int get_pos_DIR(struct task_struct *t) 
{
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		if(t->dir_pages_baseAddr == (page_table_entry*) &dir_pages[i]) {
			return i; 
		}			
		
	}	
	return -1;
}

	//ALLOCATE NUEVO, SE QUEDA EN EL TEST[15] DEL FORK
 
int allocate_DIR(struct task_struct *t) 
{
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		if(!dir_busy[i]) {
			t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[i];
			dir_busy[i]++;
			return i; 
		}			
		
	}	
}

/*
	//ALLOCATE VIEJO, PASA EL FORK
int allocate_DIR(struct task_struct *t) 
{
	int pos;

	pos = ((int)t-(int)task)/sizeof(union task_union);

	if(!dir_busy[pos]) {
		t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 
		dir_busy[pos]++;
	}
		
	return pos;

}

*/


void cpu_idle(void)
{
	__asm__ __volatile__("sti": : :"memory");
	while(1)
	{
		;
	}
}

void init_readyqueue()
{
	INIT_LIST_HEAD(&readyqueue);
}

void init_keyboardqueue()
{
	INIT_LIST_HEAD(&keyboardqueue);
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

void init_semaphores()
{
	int i;
	for (i = 0; i < SEMAPHORES_SIZE; ++i)
	{
		semaphores[i].owner = -1;
	}
}

void init_idle (void)
{ 
	
	//printk("IDLE");
	
	struct list_head *first =  list_first(&freequeue);		//agafar el primer element de la frequeue
	list_del(first);										//el proces ja no esta en la frequeue
	
	struct task_struct *pcb = list_head_to_task_struct(first);

	pcb->PID = 0;
	pcb->quantum_rr = DEFAULT_QUANTUM;

	allocate_DIR(pcb);		//inicialitza el camp dir_pages_baseAddr per guardar l'espai d'adreces

	union task_union *task_u = (union task_union *)pcb;

	task_u->stack[KERNEL_STACK_SIZE-1] = (unsigned long)&(cpu_idle);		//adreça de retorn 
	task_u->stack[KERNEL_STACK_SIZE-2] = 0;					//ebp
	task_u->task.kernel_esp = (unsigned long)&(task_u->stack[KERNEL_STACK_SIZE-2]);
	//task_u.task.kernel_esp = KERNEL_ESP(&task_u);			//esp
	pcb->program_break = HEAP_START*PAGE_SIZE;
	idle_task = pcb;
}

void init_task1(void)
{
	
	struct list_head *first =  list_first(&freequeue);		//agafar el primer element de la frequeue
	list_del(first);										//el proces ja no esta en la frequeue
	
	struct task_struct *pcb = list_head_to_task_struct(first);

	pcb->PID = 1;
	
	
    pcb->quantum_rr = DEFAULT_QUANTUM;

    pcb->state=ST_RUN;

    remaining_quantum = pcb->quantum_rr;
  

	allocate_DIR(pcb);		//inicialitza el camp dir_pages_baseAddr per guardar l'espai d'adreces

	set_user_pages(pcb);

	union task_union *task_u = (union task_union *)pcb;

	tss.esp0 = KERNEL_ESP(task_u);

	set_cr3(pcb->dir_pages_baseAddr);
	
	pcb->program_break = HEAP_START*PAGE_SIZE;
	


}

void init_stats(struct stats * s)
{
	s->user_ticks = 0;
	s->system_ticks = 0;
	s->blocked_ticks = 0;
	s->ready_ticks = 0;
	s->elapsed_total_ticks = get_ticks();
	s->total_trans = 0;
	s->remaining_ticks = get_ticks();
}


void update_stats(unsigned long *v, unsigned long *elapsed)
{
	unsigned long ticks = get_ticks();
	*v += ticks - *elapsed;
	*elapsed = ticks;
}


void init_sched()
{

	init_freequeue();
	init_readyqueue();
	init_keyboardqueue();
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

void inner_task_switch(union task_union *new)
{

	tss.esp0 = (int)&(new->stack[KERNEL_STACK_SIZE]);

	//FLUSH
	set_cr3(new->task.dir_pages_baseAddr);

	//movl %ebp, current()->task.kernel_esp
	__asm__ __volatile__(
		"movl %%ebp, %0\n\t"
		:"=g" (current()->kernel_esp)
		:
	);

	//movl new->task.kernel_esp, %esp;
	__asm__ __volatile__(
		"movl %0, %%esp\n\t"
		: 
		:"g" (new->task.kernel_esp)
	);

	__asm__ __volatile__(
		"popl %ebp\n\t"
		"ret\n\t"
	);
		
}


void task_switch(union task_union *new)
{
	__asm__ __volatile__ (
  		"pushl %esi\n\t"
		"pushl %edi\n\t"
		"pushl %ebx\n\t"
	);

	inner_task_switch(new);

	__asm__ __volatile__ (
		"popl %ebx\n\t"
  		"popl %ebx\n\t"
		"popl %edi\n\t"
		"popl %esi\n\t"
	);
}

void sched_next_rr()
{
	struct task_struct * task;
	if(list_empty(&readyqueue)){
		//printk(" EMPTY ");
		task = idle_task;
	}else{
		//printk(" NO EMPTY ");
		struct list_head * list = list_first(&readyqueue);
		task =  list_head_to_task_struct(list);
		//if(task->PID != 0) printk(" OJOOOOOOOOOOOOOOOOOOOOOOOOO ");
		list_del(list);		
	}

	task->state = ST_RUN;
	remaining_quantum = get_quantum(task);
	
  update_stats(&(current()->statistics.system_ticks), &(current()->statistics.elapsed_total_ticks));
  update_stats(&(task->statistics.ready_ticks), &(task->statistics.elapsed_total_ticks));
  task->statistics.total_trans++;
	
	//printk("BBB");
	task_switch(task);
	//printk("AAA");

	/*if(list_empty(&readyqueue))
	 printk(" empty ");
	 else 
	 printk(" no empty ");*/
}

void update_process_state_rr(struct task_struct *t, struct list_head *dest)
{
	//si l'estat del proces actual no es running, es borra de la llista en la que esta
	//printk(" - ");
	if(t->state != ST_RUN)
		list_del(&(t->list));

	if(dest == NULL){
		t->state = ST_RUN;
	}
	else{
		list_add_tail(&(t->list),dest);
		if(dest != &readyqueue)
			t->state = ST_BLOCKED;
		else{
			update_stats(&(t->statistics.system_ticks), &(t->statistics.elapsed_total_ticks));
			t->state = ST_READY;
		}
	}
}

int needs_sched_rr()
{
  if ((remaining_quantum==0)&&(!list_empty(&readyqueue))) return 1;
  if (remaining_quantum==0) return 1;
  return 0;
}

void update_sched_data_rr()
{
	--remaining_quantum;
}

void schedule()
{
	update_sched_data_rr();
	if(needs_sched_rr()){
		update_process_state_rr(current(),&readyqueue);
		sched_next_rr();
	}
}


int get_quantum(struct task_struct * t)
{
	return t->quantum_rr;
}

void set_quantum(struct task_struct * t, int new_quantum)
{
	t->quantum_rr = new_quantum;
}
