/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;
int zeos_ticks = 0;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','�','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','�',
  '\0','�','\0','�','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

//per inicialitzar una posicio en la IDT
//int vector -> entrada en la idt
//void (*handler)() -> adre�a del handler que s'encarrega de l'exepcio
//int maxAccessibleFromPL 0-> kernel 3-> user
void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void system_call_handler();
void keyboard_handler();
void clock_handler();

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(33, keyboard_handler, 0);
  setTrapHandler(0x80, system_call_handler, 3);

  set_idt_reg(&idtR);
}

//SERVICE RUTINE
void clock_routine()
{
	zeos_show_clock();
	zeos_ticks++;
	schedule();
  //if (zeos_ticks == 1030)
    //task_switch(list_head_to_task_struct(list_first(&readyqueue)));
}

void keyboard_routine()
{

  unsigned char input = inb( 0x60 );
  unsigned char c;


  if ( input&0x80 ){
    //printk("KEYBOARD_ROUTINE");
    c = char_map[input & 0x7f];
    circularbuffer[(posicionInicialParaLeer+bytesCircularBufferOcupados)%512] = c;
	bytesCircularBufferOcupados++;
    if(c != '\0') 
      printc_xy(70,2, c );
    else
      printc_xy(70,2,'C');
      
      if (!list_empty(&keyboardqueue)) {
		  
		  
		  
	/*	  if(list_empty(&readyqueue))
	 printk(" empty2 ");
	 else 
	 printk(" no empty2 ");
		  */
		  
		  
		  
		  
		  
		//  printk("NOOOOOOO");
		struct list_head *pcb_list =  list_first(&keyboardqueue);		//agafar el primer element de la keyboardqueue
		struct task_struct * pcb= list_head_to_task_struct(pcb_list);
		pcb->state = ST_READY;
		list_del(pcb_list);
		list_add_tail(pcb_list,&readyqueue);
		//sched_next_rr();
		//printk("YEAH");
		
		
	/*	if(list_empty(&readyqueue))
	 printk(" empty3 ");
	 else 
	 printk(" no empty3 ");*/
	}
  }
  

  
  
  
  
  
}

