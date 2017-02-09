#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void exit_error(char *err_msg) {
  fprintf(stderr, "%s", err_msg);
  exit(1); 
}

void safe_print(char *msg, pthread_mutex_t *mu) {
  pthread_mutex_lock(mu);
  printf("%s", msg);
  pthread_mutex_unlock(mu);  
}
