/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#include <stats.h>

#define LECTURA 0
#define ESCRIPTURA 1

extern int zeos_ticks;
int PIDs = 1;


int sys_sem_init (int n_sem, unsigned int value)
{
	if(n_sem >= SEMAPHORES_SIZE || n_sem < 0)
		return -ENXIO;
	else if(semaphores[n_sem].value != -1)
		return -EINVAL;

	semaphores[n_sem].value = value;
	semaphores[n_sem].owner = current()->PID;
	INIT_LIST_HEAD(&semaphores[n_sem].blocked_processes);
	return 0;
}

int check_fd(int fd, int permissions)
{
	if (fd!=1) return -EBADF; /* Bad file number */
	if (permissions!=ESCRIPTURA) return -EACCES; /* Permission denied */
	return 0;
}

void user_to_system(void)
{
  update_stats(&(current()->statistics.user_ticks), &(current()->statistics.elapsed_total_ticks));
}

void system_to_user(void)
{
  update_stats(&(current()->statistics.system_ticks), &(current()->statistics.elapsed_total_ticks));
}

int sys_ni_syscall()
{
	return -ENOSYS;
}

int sys_gettime()
{
	return zeos_ticks;
}

int sys_getpid()
{
	return current()->PID;
}

int ret_from_fork()
{
	return 0;
}

int sys_write(int fd, char * buffer, int size)
{
	// fd: file descriptor, in this delivery it must always be 1
	// buffer: pointer to the bytes
	// size: number of bytes
	// return a negative number in case of error (specifying the kind of error)
	// 	and the number of bytes written if OK 
	char localBuff [64];
	int res = -1;
	int check = check_fd(fd, ESCRIPTURA);
	
	if(check) 
		return check;
	
	if(size < 0) 
		return -EINVAL;
	
	if(buffer != NULL) {
		copy_from_user(buffer, buffer, size);
		res = sys_write_console(buffer,size);
	}
	return res;
}

int sys_clone(void (*function)(void), void *stack)
{
	// function: starting address of the function to be executed by the new process
	// stack   : starting address of a memory region to be used as a stack
	//
	struct list_head *child =  list_first(&freequeue);		//agafar el primer element de la freequeue
	list_del(child);										//el proces ja no esta en la freequeue
	
	struct task_struct *pcb_child = list_head_to_task_struct(child);
	union task_union * task_union_child = (union task_union *)pcb_child;

	copy_data( current(), task_union_child, sizeof(union task_union) );	//copiar el task union del pare en el fill
	//
	pcb_child->PID = ++PIDs;
	
	task_union_child->stack[KERNEL_STACK_SIZE-18] = (int)&ret_from_fork;
  	task_union_child->stack[KERNEL_STACK_SIZE-19] = 0;
  	pcb_child->kernel_esp = (int)&task_union_child->stack[KERNEL_STACK_SIZE-19];
	
	
	allocate_DIR(task_union_child);
	
	return pcb_child->PID;
}

int sys_fork() 
{
	// creates the child process
	
	//2)
	//a)
	if(list_empty(&freequeue)) 
		return -ENOMEM;
	
	//b)
	struct list_head *child =  list_first(&freequeue);		//agafar el primer element de la freequeue
	list_del(child);										//el proces ja no esta en la freequeue
	
	struct task_struct *pcb_child = list_head_to_task_struct(child);
	union task_union * task_union_child = (union task_union *)pcb_child;

	copy_data( current(), task_union_child, sizeof(union task_union) );	//copiar el task union del pare en el fill
	
	//c)
	allocate_DIR(pcb_child);				//inicialitza el camp dir_pages_baseAddr per guardar l'espai d'adreces
	
	//e)
	//i)
	page_table_entry * PT_child = get_PT(pcb_child);
	page_table_entry * PT_parent = get_PT(current());
	//ii)
	int page, i_free_frame;
	
	//buscar memoria fisica per data+stack
	for (page=PAG_LOG_INIT_DATA; page < NUM_PAG_DATA+PAG_LOG_INIT_DATA; page++){
		i_free_frame = alloc_frame(); //buscar pagines fisiques on mapejar les pagines logiques per data+stack del fill

		if(i_free_frame != -1){
			set_ss_pag(PT_child, page, i_free_frame);				
		}
		else{
			//si no es pot tot s'ha d'alliberar tota la memoeria que s'havia reservat fins ara 
			int page2;
			for (page2 = PAG_LOG_INIT_DATA; page2 < page; page2++){
				free_frame(get_frame(PT_child,page2));
				del_ss_pag(PT_child,page2);
			}
			list_add_tail(child,&freequeue);

			return -ENOMEM;
		}
	}

	//les pagines de kernel es comparteixen, nomes s'ha de mapejar les adreces
	for (page = 0; page < NUM_PAG_KERNEL; page++){
		set_ss_pag(PT_child, page, get_frame(PT_parent, page));
	}

	//les pagines de code es comparteixen, nomes s'ha de mapejar les adreces
	for (page = PAG_LOG_INIT_CODE; page < NUM_PAG_CODE+PAG_LOG_INIT_CODE; page++){
		set_ss_pag(PT_child, page, get_frame(PT_parent, page));
	}

	//les pagines de data no es comparteixen, s'ha de copiar i mapejar les adreces
	for (page = NUM_PAG_KERNEL+NUM_PAG_CODE; page < NUM_PAG_KERNEL+NUM_PAG_CODE+NUM_PAG_DATA; page++){
		set_ss_pag(PT_parent, NUM_PAG_DATA+page, get_frame(PT_child, page));
		copy_data((void*)(page<<12), (void*)((page+NUM_PAG_DATA)<<12), PAGE_SIZE);
		del_ss_pag(PT_parent, NUM_PAG_DATA+page);		
	}

	set_cr3(get_DIR(current()));

	pcb_child->PID = ++PIDs;


	task_union_child->stack[KERNEL_STACK_SIZE-18] = (int)&ret_from_fork;
  	task_union_child->stack[KERNEL_STACK_SIZE-19] = 0;
  	pcb_child->kernel_esp = (int)&task_union_child->stack[KERNEL_STACK_SIZE-19];


	init_stats(&pcb_child->statistics);

	pcb_child->state = ST_READY;

	list_add_tail(&(pcb_child->list), &readyqueue);

	return pcb_child->PID;
}

void sys_exit()
{

	struct task_struct * pcb = current();
	page_table_entry * PT = get_PT(pcb);

	int page;
	for (page = PAG_LOG_INIT_DATA; page < NUM_PAG_DATA+PAG_LOG_INIT_DATA; page++){
		free_frame(get_frame(PT,page));
		del_ss_pag(PT,page);
	}

	list_add_tail(&(pcb->list), &freequeue);
	pcb->PID = -1;
	sched_next_rr();

}

extern int remaining_quantum;

void sys_get_stats(int pid, struct stats *st)
{
  int i;
  
  if (!access_ok(VERIFY_WRITE, st, sizeof(struct stats))) return -EFAULT; 
  
  if (pid<0) return -EINVAL;
  for (i=0; i<NR_TASKS; i++)
  {
    if (task[i].task.PID==pid)
    {
      task[i].task.statistics.remaining_ticks= remaining_quantum;
      copy_to_user(&(task[i].task.statistics), st, sizeof(struct stats));
      return 0;
    }
  }
	return -ESRCH; /*ESRCH */	
}
