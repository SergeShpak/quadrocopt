#include <stdlib.h>

#include "include/threading_stuff.h"

ThreadConditionPack *init_thread_cond_pack(pthread_cond_t *var, 
                            pthread_mutex_t *cond_mu) {
  ThreadConditionPack *result 
    = (ThreadConditionPack *) malloc(sizeof(ThreadConditionPack));
  result->cond_var = var;
  result->mutex_to_use = cond_mu;
  result->cond_to_verify = 0;
  return result;
}

void free_thread_cond_pack(ThreadConditionPack *thread_cond_pack) {
  free(thread_cond_pack);
}

void set_cond_to_verify_to_true(ThreadConditionPack *cond_pack) {
  cond_pack->cond_to_verify = 1;
}

void set_cond_to_verify_to_false(ThreadConditionPack *cond_pack) {
  cond_pack->cond_to_verify = 0;
}

int is_cond_false(ThreadConditionPack *cond_pack) {
  return !(cond_pack->cond_to_verify);
}
