#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semWait(){
 
  // do stuff :)

  /*For get the sem_d we need the fd from the PCB of the running process */
  int fd = running -> syscall_args[0];
  SemDescriptor* semdesc = SemDescriptorList_byFd(&running->sem_descriptors, fd);
  if (!semdesc){
    running -> syscall_retvalue = DSOS_ESEMWAIT_SEMD_NOT_IN_PROCESS;    /*The proc is already in wait or close*/
    return;
  }

  /*Decrease the count of s. If the count is negative insert the running proc in wait list */
  Semaphore* s = semdesc->semaphore;
  assert(s);
  s->count--;
  PCB* pcb = running;
  if (s->count < 0){   /*The resources are finished for this semaphore => add the proc in list wait*/
    SemDescriptorPtr* semdesc_ptr = SemDescriptorPtr_alloc(semdesc);
    assert(semdesc_ptr);
    List_insert(&s->waiting_descriptors, s->waiting_descriptors.last, (ListItem*)semdesc_ptr);
    running->status = Waiting;
    List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
    running = (PCB*)List_detach(&ready_list, ready_list.first);
    running -> status = Running;
  }

  /*Set the ret value of syscall */
  pcb->syscall_retvalue = 0;

}
