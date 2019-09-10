#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semOpen(){
  
  //Check if the running process has too much descriptors on its list
  if(running->sem_descriptors.size >=  MAX_NUM_SEMDESCRIPTORS_PER_PROCESS) {  //defined in disastrOS_constants (disastrOS_globals -> disastrOS_pcb)
    running->syscall_retvalue = DSOS_ESOPEN_OUT_OF_BOUND_SEMDESCRIPTORS;
    return;
  }

  //Check if the syscall value in the calling process pcb is admissible
  int ID = running->syscall_args[0];
  if(ID < 0) {
    running->syscall_retvalue = DSOS_ESEMOPEN_SEMNUM_VALUE;
    return;
  }

  //Check if the semaphore is already open, in that case we add it to the global list
  Semaphore* sem0 = SemaphoreList_byId((SemaphoreList*) &semaphores_list, ID);
  if(!sem0) {
    sem0 = Semaphore_alloc(ID, 1);
    assert(sem0);
    List_insert(&semaphores_list, semaphores_list.last, (ListItem*) sem0);
  }

  //Alloc the SemDescriptor for sem associated with the running process
  SemDescriptor* semdesc = SemDescriptor_alloc(running->last_sem_fd, sem0, running);
  assert(semdesc);

  //Update last_sem_fd for the next running process
  running->last_sem_fd++;

  //Alloc Pointer of SemDescriptor for semdesc
  SemDescriptorPtr* semdesc_ptr = SemDescriptorPtr_alloc(semdesc);
  assert(semdesc_ptr);

  //Link semdesc_ptr with semdesc
  semdesc->ptr = semdesc_ptr;

  //Add semdesc to the SemDescriptors list of the running process
  List_insert(&running->sem_descriptors, running->sem_descriptors.last, (ListItem*) semdesc);

  //Add semdesc_ptr to the descriptors_list of sem0
  List_insert(&sem0->descriptors, sem0->descriptors.last, (ListItem*) semdesc_ptr);

  //Set the return value of the syscall
  running->syscall_retvalue = semdesc->fd;
  
  return;

}
