#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

void internal_semClose(){
  // do stuff :)

  /*Get the file descr of SemDescriptor that the process wants close */
  int fd = running -> syscall_args[0];

  /*Take SemDescriptor to the list opened by running proc */
  SemDescriptor* sem_desc = SemDescriptorList_byFd(&running -> sem_descriptors, fd);
  if (!sem_desc){
    running -> syscall_retvalue = DSOS_ESEMCLOSE_SEMD_NOT_IN_PROCESS;
    return;
  }

  /*Delete sem_desc from  the list of SemDescr of the proc*/
  List_detach(&running -> sem_descriptors, (ListItem*)sem_desc);

  /*Get sem from sem_descr */
  Semaphore* s = sem_desc -> semaphore;
  assert(s);

  /*Delete sem_desc_ptr from the list */
  SemDescriptorPtr* sem_desc_ptr = (SemDescriptorPtr*)List_detach(&s->descriptors, (ListItem*)sem_desc->ptr);
  assert(sem_desc_ptr);

  /*Free the memory */
  SemDescriptorPtr_free(sem_desc_ptr);
  SemDescriptor_free(sem_desc);

  /*Check if there aren't other processes on semaphore, if it's OK remove it from the struct and free the mem */
  if (s->descriptors.size == 0){
    s = (Semaphore*)List_detach(&semaphores_list, (ListItem*)s);
    assert (s);
    Semaphore_free(s);
  }

  running -> syscall_retvalue = 0;


}
