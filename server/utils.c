#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void exit_error(char *err_msg) {
  fprintf(stderr, "%s", err_msg);
  exit(1); 
}

void safe_print(char *msg, pthread_mutex_t *mu) {
  pthread_mutex_lock(mu);
  printf("%s", msg);
  pthread_mutex_unlock(mu);  
}

int get_random_int(int low, int high) {
  int r_int = rand() % (high - low) + low;
  return r_int;
}

void rand_sleep(int low, int high) {
  int period_rand = get_random_int(low, high);
  sleep(period_rand);
}
