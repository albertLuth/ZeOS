/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;

int write(int fd, char *buffer, int size)
{
  int res;
  __asm__ volatile( 
    "int $0x80"              //  interrupcio 0x80, crida al sistema
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
  }
}
