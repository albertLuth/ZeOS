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

#define LECTURA 0
#define ESCRIPTURA 1

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; /* Bad file number */
  if (permissions!=ESCRIPTURA) return -EACCES; /* Permission denied */
  return 0;
}

int sys_ni_syscall()
{
	return -ENOSYS;
}

int sys_write(int fd, char * buffer, int size)
{
	// fd: file descriptor, in this delivery it must always be 1
	// buffer: pointer to the bytes
	// size: number of bytes
	// return a negative number in case of error (specifying the kind of error)
	// 	and the number of bytes written if OK 
	int res = -1;
	int check = check_fd(fd, ESCRIPTURA);
	if(check) return check;
	else if(buffer != NULL && size >= 0){
		copy_from_user(buffer, buffer, size);
		res = sys_write_console(buffer,size);
	}

	return res;
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}
