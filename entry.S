/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>

/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %edx; \
      pushl %ecx; \
      pushl %ebx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

#define RESTORE_ALL \
      popl %ebx; \
      popl %ecx; \
      popl %edx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \
      popl %eax; \
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs; 

#define EOI \
      movb $0x20, %al; \
      outb  %al, $0x20; 
      
      
      
/***************** handlers *****************/ 


ENTRY(system_call_handler)
      SAVE_ALL                            // save the current context
		pushl %eax;
		call user_to_system;
		popl %eax;
      cmpl $0, %eax;
      jl err                              // if it is, jump to return an error
      cmpl $MAX_SYSCALL, %eax             // is syscall greater than MAX_SYSCALL?
      jg err                              // if it is, jump to return an error
      call *sys_call_table(,%eax,0x04)    // execute the system call indexed by the register eax
      jmp fin                             // finish
err:
      movl $-ENOSYS, %eax                 // move to eax the ENOSYS error
fin:
      movl %eax, 0x18(%esp)               // change the eax value in the stack
		pushl %eax;
		call system_to_user;
		popl %eax;
      RESTORE_ALL                         // restore the context
      iret                                // returns program control from an exception or interrupt handler to a program that was interrupted 


ENTRY(keyboard_handler)
      SAVE_ALL
        pushl %eax;
		call user_to_system;
		popl %eax;
      EOI
      call keyboard_routine
		pushl %eax;
		call system_to_user;
		popl %eax;
      RESTORE_ALL                         
      iret


ENTRY(clock_handler)
      SAVE_ALL
        pushl %eax;
		call user_to_system;
		popl %eax;
      EOI
      call clock_routine
      	pushl %eax;
		call system_to_user;
		popl %eax;
      RESTORE_ALL                         
      iret
      
      
