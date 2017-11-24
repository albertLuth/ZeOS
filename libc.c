/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;


//n_sem: identifier of the semaphore to be initialized
//value: initial value of the counter of the semaphore
//returns: -1 if error, 0 if OK
int sem_init (int n_sem, unsigned int value)
{
    int res;
  __asm__ volatile( 
    "int $0x80"                 //  interrupcio 0x80, crida al sistema
    : "=a" (res),               //  el resultat de %eax es guarda en res
      "+b" (n_sem),             //  passar el parametre n_sem per %ecx
      "+c" (value)              //  passar el parametre value per %ebx
    : "a" (21)                  //  %eax = 21, la crida al sistema sys_sem_init
    );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}


//n_sem: identifier of the semaphore
//return: -1 if error, 0 if OK
int sem_wait (int n_sem)
{
  int res;
  __asm__ volatile( 
    "int $0x80"                 //  interrupcio 0x80, crida al sistema
    : "=a" (res),               //  el resultat de %eax es guarda en res
      "+b" (n_sem)              //  passar el parametre n_sem per %ecx
    : "a" (22)                  //  %eax = 22, la crida al sistema sys_sem_wait
    );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}

//n_sem: identifier of the semaphore
//returns: -1 if error, 0 if OK
int sem_signal (int n_sem)
{
  int res;
  __asm__ volatile( 
    "int $0x80"                 //  interrupcio 0x80, crida al sistema
    : "=a" (res),               //  el resultat de %eax es guarda en res
      "+b" (n_sem)              //  passar el parametre n_sem per %ecx
    : "a" (23)                  //  %eax = 23, la crida al sistema sys_sem_signal
    );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}


//n_sem: identifier of the semaphore to destroy
//returns: -1 if error, 0 if OK
int sem_destroy (int n_sem)
{
    int res;
  __asm__ volatile( 
    "int $0x80"                 //  interrupcio 0x80, crida al sistema
    : "=a" (res),               //  el resultat de %eax es guarda en res
      "+b" (n_sem)              //  passar el parametre n_sem per %ecx
    : "a" (24)                  //  %eax = 24, la crida al sistema sys_sem_destroy
    );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}



int write(int fd, char *buffer, int size)
{
  int res;
  __asm__ volatile( 
    "int $0x80"             //  interrupcio 0x80, crida al sistema
    : "=a" (res),           //  el resultat de %eax es guarda en res
      "+b" (fd),            //  passar el parametre fd per %ebx
      "+c" (buffer),        //  passar el parametre bufer per %ecx
      "+d" (size)           //  passar el parametre size per %edx
    : "a" (4)               //  %eax = 4, la crida al sistema write
    );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}

int clone(void (*function)(void), void *stack)
{
  int res;
  __asm__ volatile( 
    "int $0x80"             	  //  interrupcio 0x80, crida al sistema
    : "=a" (res),               //  el resultat de %eax es guarda en res
      "+b" (function),          //  passar el parametre function per %ecx
      "+c" (stack)	            //  passar el parametre stack per %ebx
    : "a" (19)                  //  %eax = 19, la crida al sistema clone
    );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}

int gettime()
{
  int res;
  __asm__ volatile( 
    "int $0x80"              //  interrupcio 0x80, crida al sistema
    : "=a" (res)             //  el resultat de %eax es guarda en res
    : "a" (10)               //  %eax = 10, la crida al sistema write
    );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}

int fork()
{
  int res;
  __asm__ volatile( 
    "int $0x80"              //  interrupcio 0x80, crida al sistema
    : "=a" (res)             //  el resultat de %eax es guarda en res
    : "a" (2)               //  %eax = 2, la crida al sistema fork
    );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}

void exit()
{
  __asm__ volatile( 
    "int $0x80"              //  interrupcio 0x80, crida al sistema
    : 
    : "a" (1)               //  %eax = 2, la crida al sistema fork
    );
}

int getpid()
{
  int res;
  __asm__ volatile( 
    "int $0x80"              //  interrupcio 0x80, crida al sistema
    : "=a" (res)             //  el resultat de %eax es guarda en res
    : "a" (20)               //  %eax = 20, la crida al sistema getpid
  );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}

int get_stats(int pid, struct stats *st)
{
  int res;
  __asm__ volatile( 
    "int $0x80"                 //  interrupcio 0x80, crida al sistema
    : "=a" (res)                //  el resultat de %eax es guarda en res
    : "a" (35),"b"(pid),"c"(st) //  %eax = 20, la crida al sistema getpid
  );

  if(-125 <= res && res < 0){
    errno = -res;
    res = -1;
  }
  return res;
}

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

void perror()
{
  switch(errno)
  {
    case ENOSYS:
      write(1, "Function not implemented\n", 25);
      break;
      
    case EINVAL:
      write(1, "Invalid argument\n", 17);
      break;
      
    case EPERM:
      write(1, "Operation not permitted\n", 24);
      break;
      
    case ENOMEM:
      write(1, "Out of memory\n", 14);
      break;

    case ENXIO:
      write(1, "No such device or address\n", 26);
      break;
    case EACCES:
      write(1, "Permission denied\n", 18);
      break;
  }
}


