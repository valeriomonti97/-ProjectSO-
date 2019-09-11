#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semPost(){

  // do stuff :)

  /* Get the fd from the PCB of the running proc and use it for the semdesc */
  int fd = running -> syscall_args[0];
  SemDescriptor* semdesc = SemDescriptorList_byFd(&running -> sem_descriptors, fd);
  if (!semdesc){
    running -> syscall_retvalue = DSOS_ESEMPOST_SEMD_NOT_IN_PROCESS; /*The proc is already closed or in running or not exist*/
    return;
  }

  /*Increase the count of s*/
  Semaphore* s = semdesc -> semaphore;
  assert(s);
  s->count++;
  if (s->count <= 0){                /* If its value is negative or 0, the proc in waiting list can be ready*/
    SemDescriptorPtr* semdesc_ptr = (SemDescriptorPtr*)List_detach(&s->waiting_descriptors, s->waiting_descriptors.first);
    PCB* pcb = semdesc_ptr->descriptor->pcb;
    SemDescriptorPtr_free(semdesc_ptr);
    pcb->status = Ready;
    List_detach(&waiting_list, (ListItem*)pcb);
    List_insert(&ready_list, ready_list.last, (ListItem*)pcb);
  }

  /* Set the ret value of syscall */
  running->syscall_retvalue = 0;

}
