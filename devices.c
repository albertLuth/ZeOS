#include <io.h>
#include <utils.h>
#include <list.h>

//
/*
    - warnings 
    + passa E2 
    - sys_get_stats: hauria de retornar un 'int' 
    - sem_wait: hauria de distingir el motiu de desbloqueig: signal o destroy 
    - sem_wait: s'hauria de bloquejar si el comptador es 0 
    - allocate_DIR: usa possicio dins vector de tasques 
    - get_pos_DIR: idem 
    - exit: independentment de si cal alliberar frames o no, cal marcar PCB com a lliure 
    - clone: ha de comprovar que pugui crear threads nous 
    - task_switch: si el proper PCB a executar és un thread del procés actual NO cal canviar l'espai d'adreces 
*/

// Queue for blocked processes in I/O 
struct list_head blocked;

int sys_write_console(char *buffer,int size)
{
  int i;
  
  for (i=0; i<size; i++)
    printc(buffer[i]);
  
  return size;
}

