#include <libc.h>

char buff[24];

int pid;

int add(int par1, int par2){
	__asm__("movl 0x8(%ebp), %eax");
	__asm__("addl 0xc(%ebp), %eax");
}

long inner(long n)
{
	int i;
	long suma;
	suma = 0;
	for (i = 0; i < n; ++i) suma = add(suma,i);
	return suma;
}

long outer(long n)
{
	int i;
	long acum;
	acum = 0;
	for (i = 0; i < n; ++i) acum = add(acum,inner(i));
	return acum;
}


int __attribute__ ((__section__(".text.main")))
  main()
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

	
    long count, acum;
    count = 76;
    acum = 0;
    acum = outer(count);
    while(1);
    return 0;	
}
