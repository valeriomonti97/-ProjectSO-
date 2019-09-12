#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include "assert.h"
#include "disastrOS_globals.h"
#include "disastrOS.h"
#include <stdlib.h>

//error_helper
void error_helper(int i){
  fprintf(stderr, "Error in ChildFunction with code: %d", i);
  abort();
}

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=0;
  int mode=0;
  int ret;
  int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n", fd);
  printf("PID: %d, terminating\n", disastrOS_getpid());

  //TEST 1 -> semOpen & semClose

  printf("****************** TEST 1 ******************\n");
  for (int i = 0; i < disastrOS_getpid()+1; i++){
    ret = disastrOS_mysemOpen(i);
    if (ret < 0){
      error_helper(ret);
    }
  }
  disastrOS_printStatus();
  for (int i = 0; i < disastrOS_getpid(); i++){
    ret = disastrOS_mysemClose(i);
  if (ret){
      error_helper(ret);
    }
  }
  disastrOS_printStatus();

  //TEST 2 -> all semaphores operations

  printf("****************** TEST 2 ******************\n");
  for (int i = 0; i < disastrOS_getpid()+1; ++i){
    fd = disastrOS_mysemOpen(i);
    if (fd < 0){
        error_helper(ret);
    }
    ret = disastrOS_mysemWait(fd);
    if (ret){
        error_helper(ret);
    }
    disastrOS_preempt();
    ret = disastrOS_mysemPost(fd);
    if (ret){
        error_helper(ret);
    }
    disastrOS_printStatus();
    ret = disastrOS_mysemClose(fd);
    if (ret){
        error_helper(ret);
    }
  }

  //TEST 3

  printf("****************** TEST 3 ******************\n");
  fd = disastrOS_mysemOpen(sh_semID);
  if (fd < 0){
    error_helper(ret);
  }
  ret = disastrOS_mysemWait(fd);
  if (ret){
    error_helper(ret);
  }
  disastrOS_preempt();
  ret = disastrOS_mysemPost(fd);
  if (ret){
    error_helper(ret);
  }
  disastrOS_printStatus();
  ret = disastrOS_mysemClose(fd);
  if (ret){
    error_helper(ret);
  }
  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {
  disastrOS_printStatus();
  printf("hello, I am init and I just started\n");
  disastrOS_spawn(sleeperFunction, 0);
  

  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<10; ++i) {
    int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd);
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  sh_semID = 2*ready_list.size;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){ 
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }
  printf("shutdown!");
  disastrOS_shutdown();
}

int main(int argc, char** argv){
  char* logfilename=0;
  if (argc>1) {
    logfilename=argv[1];
  }
  // we create the init process processes
  // the first is in the running variable
  // the others are in the ready queue
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
