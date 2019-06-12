#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"

void internal_semPost(){
  // do stuff :)

  int fd = running -> syscall_args[0];
  SemDescriptor semdesc = SemDescriptorList_byFd(&running -> sem_descriptors, fd);
  if (!semdesc){
    running -> syscall_retvalue = DSOS_ESEMPOST_SEMD_NOT_IN_PROCESS;
    return;
  }

  Semaphore* s = semdesc -> semaphore;
  assert(sem);
  sem->count++;
  if (sem->count <= 0){
    SemDescriptorPtr* semdesc_ptr = (SemDescriptorPtr*)List_detach(&s->waiting_descriptors, s->waiting_descriptors.first);
    PCB* pcb = semdesc_ptr->descriptor->pcb;
    SemDescriptorPtr_free(semdesc_ptr);
    pcb->status = Ready;
    List_detach(&waiting_list, (ListItem*)pcb);
    List_insert(&ready_list, ready_list.last, (ListItem*)pcb);
  }

  running->syscall_retvalue = 0;

}
