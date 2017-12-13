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

void *sys_sbrk(int increment) 
{	
	struct task_struct *pcb = current();
	//void * program_break = current()->program_break;
	
	
	if (increment > 0) {
		
		
		
	}
	else if (increment < 0) {
		
		
	} 
	pcb->program_break += increment;  
	
	return pcb->program_break;
}

int sys_sem_init (int n_sem, unsigned int value)
{	
	printk("INIT");
	if(n_sem >= SEMAPHORES_SIZE || n_sem < 0)
		return -EINVAL;
	else if(semaphores[n_sem].owner != -1)
		return -EBUSY;

	semaphores[n_sem].value = value;
	semaphores[n_sem].owner = current()->PID;
	INIT_LIST_HEAD(&semaphores[n_sem].blocked_processes);
	return 0;
}

int sys_sem_wait(int n_sem)
{
	printk("WAIT");
	if(n_sem >= SEMAPHORES_SIZE || n_sem < 0)
		return -EINVAL;
	else if(semaphores[n_sem].owner == -1)
		return -EINVAL;
	else if(semaphores[n_sem].value < 0){
		current()->state = ST_BLOCKED;
		list_add_tail(&current()->list, &semaphores[n_sem].blocked_processes);
		sched_next_rr();
	}
	else semaphores[n_sem].value--;	

	return 0;
}

int sys_sem_signal(int n_sem)
{
	printk("SIGNAL");
	if(n_sem >= SEMAPHORES_SIZE || n_sem < 0)
		return -EINVAL;
	else if(semaphores[n_sem].owner == -1)
		return -EINVAL;
	else if(list_empty(&semaphores[n_sem].blocked_processes))
		semaphores[n_sem].value++;
	else{
		struct list_head *p = list_first(&semaphores[n_sem].blocked_processes);
		list_del(p); //s'elimina el proces de la llista de bloquejats, es debloqueja
		list_add_tail(p, &readyqueue);
		struct task_struct *pcb_p = list_head_to_task_struct(p);
		pcb_p->state = ST_READY;
	}

	return 0;
}

int sys_sem_destroy(int n_sem)
{
	printk("DESTROY");
	if(n_sem >= SEMAPHORES_SIZE || n_sem < 0)
		return -EINVAL;
	else if(semaphores[n_sem].owner != current()->PID && semaphores[n_sem].owner != -1)
		return -EPERM;	
	else if(semaphores[n_sem].owner == current()->PID && semaphores[n_sem].owner == -1)
		return -EINVAL;
	else if(semaphores[n_sem].owner != current()->PID && semaphores[n_sem].owner == -1)
		return -EINVAL;
	else {
		semaphores[n_sem].owner = -1;
		while(!list_empty(&semaphores[n_sem].blocked_processes)){
			struct list_head *p = list_first(&semaphores[n_sem].blocked_processes);
			list_del(p);
			list_add_tail(p, &readyqueue);
			struct task_struct *pcb_p = list_head_to_task_struct(p);
			pcb_p->state = ST_READY;
		}
	}
	return 0;
}

int check_fd(int fd, int permissions)
{

	if ((fd == 1) && permissions!=ESCRIPTURA) return -EACCES; /* Permission denied */
	else if ((fd == 0) && permissions!=LECTURA) return -EACCES; /* Permission denied */
	else if (fd!=1 && fd!=0) return -EBADF; /* Bad file number */
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


int sys_read_keyboard(char *buffer,int count)
{
  int i;
  struct task_struct * pcb = current();
    
    
  //If there  are  processes  waiting  for  data  (already  blocked),  
  // then  block  the  process  at  the end of the keyboardqueue
  // and schedule the next process.
  if(!list_empty(&keyboardqueue)) {
	  pcb->state = ST_BLOCKED;
	  list_add_tail(pcb, &keyboardqueue);
	  schedule();
  } 
  else {
    //if the buffer contains all requested characters(count), 
	// copy them to the user buffer (buf) and return the total 
	// number of characters read
	if((bytesCircularBufferOcupados) >= count) {
		for (i = 0; i < count; i++) {
			buffer[i] = circularbuffer[(posicionInicialParaLeer+i)%512]; 
			//buffer[i] = 'a';
		}
		bytesCircularBufferOcupados -= count;
		posicionInicialParaLeer += count;
		
		for (i = 0; i < count; i++) {
			//printk("printk");
			printc(buffer[i]);
		}
	  
	}
	else count = 0;
	  
	  
  }
  
 
  return count;
}

int sys_read(int fd, char *buffer, int count)
{
	//read()  attempts to read up to ’count’ bytes from file descriptor ’fd’ into
	//the buffer starting at ’buf’.
	int res = -1;
	
	//if(fd == 1) printk("OK");
	
	int check = check_fd(fd, LECTURA);

	if(check)  {
		printk("CHECK");
		return check;
		
	}

	if(count <= 0) return -EINVAL;

	if(buffer != NULL) res = sys_read_keyboard(buffer,count);

	//res = hola(buffer,size);
	return res;
}






int sys_clone(void (*function)(void), void *stack)
{
	// function: starting address of the function to be executed by the new process
	// stack   : starting address of a memory region to be used as a stack
	//
	
	if (!access_ok(VERIFY_READ,function,16) || !access_ok(VERIFY_WRITE,stack,16) ){  
		  return -EFAULT;
	}
	
	struct list_head *child =  list_first(&freequeue);		//agafar el primer element de la freequeue
	struct task_struct *pcb_child = list_head_to_task_struct(child);
	union task_union * task_union_child = (union task_union *)pcb_child;
	list_del(child);


	copy_data( (union task_union*)current(), task_union_child, sizeof(union task_union) );	//copiar el task union del pare en el fill
	//	
	//
	task_union_child->stack[KERNEL_STACK_SIZE-18] = (int)&ret_from_fork;
  	task_union_child->stack[KERNEL_STACK_SIZE-19] = 0;
  	pcb_child->kernel_esp = (int)&task_union_child->stack[KERNEL_STACK_SIZE-19];
	
	task_union_child->stack[KERNEL_STACK_SIZE-5] = function;
	task_union_child->stack[KERNEL_STACK_SIZE-2] = stack;
	
		
	int pos = get_pos_DIR(pcb_child);
	
	dir_busy[pos]++;
	
	pcb_child->PID = ++PIDs;
	
	
	//allocate_DIR(task_union_child)];
		
	init_stats(&pcb_child->statistics);

	pcb_child->state = ST_READY;

	//INIT_LIST_HEAD(&(pcb_child->list));
	list_add_tail(&(pcb_child->list), &readyqueue);
	
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
	int err = allocate_DIR(pcb_child);				//inicialitza el camp dir_pages_baseAddr per guardar l'espai d'adreces
	if(err == -1) printk("ERROR");
	
	//int pos = get_pos_DIR(pcb_child);
	//dir_busy[pos]++;
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


	/////////// HEAP ////////////
	//pcb_child->program_break = HEAP_START*4096;
	pcb_child->program_break = current()->program_break;
	/*
	for (page = HEAP_START; page < current()->program_break; page++){
		set_ss_pag(PT_parent, NUM_PAG_DATA+page, get_frame(PT_child, page));
		copy_data((void*)(page<<12), (void*)((page+NUM_PAG_DATA)<<12), PAGE_SIZE);
		del_ss_pag(PT_parent, NUM_PAG_DATA+page);		
	}
	*/
	/////////////////////////////////////////////////////////////////////



	task_union_child->stack[KERNEL_STACK_SIZE-18] = (int)&ret_from_fork;
  	task_union_child->stack[KERNEL_STACK_SIZE-19] = 0;
  	pcb_child->kernel_esp = (int)&task_union_child->stack[KERNEL_STACK_SIZE-19];


	init_stats(&pcb_child->statistics);

	pcb_child->state = ST_READY;

	list_add_tail(&(pcb_child->list), &readyqueue);

	return pcb_child->PID;
}
/*
void sys_exit()
{
	int i;
	for (i = 0; i < SEMAPHORES_SIZE; ++i) {
		if (semaphores[i].owner == current()->PID) sys_sem_destroy(i);
	}

	struct task_struct * pcb = current();
	page_table_entry * PT = get_PT(pcb);


	int pos = get_pos_DIR(pcb);
	dir_busy[pos]--;
	
	if (dir_busy[pos] <= 0) {
		int page;
		for (page = PAG_LOG_INIT_DATA; page < NUM_PAG_DATA+PAG_LOG_INIT_DATA; page++){
			free_frame(get_frame(PT,page));
			del_ss_pag(PT,page);
		}
	
	}
	list_add_tail(&(pcb->list), &freequeue);
	pcb->PID = -1;
	sched_next_rr();
}*/

void sys_exit()
{
	int i;
	for (i = 0; i < SEMAPHORES_SIZE; ++i) {
		if (semaphores[i].owner == current()->PID) sys_sem_destroy(i);
	}

	struct task_struct * pcb = current();
	page_table_entry * PT = get_PT(pcb);
	

	int pos = get_pos_DIR(pcb);
	//if (dir_busy[pos] < 0) printk("OJOOOOOOOOOO");
	dir_busy[pos]--;

	if (dir_busy[pos] == 0) {
		
		int page;
		for (page = PAG_LOG_INIT_DATA; page < NUM_PAG_DATA+PAG_LOG_INIT_DATA; page++){
			free_frame(get_frame(PT,page));
			del_ss_pag(PT,page);
		}
	

		list_add_tail(&(pcb->list), &freequeue);
		pcb->PID = -1;
	}
	
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
