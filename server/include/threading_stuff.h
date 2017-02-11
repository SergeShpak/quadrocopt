#ifndef THREADING_STUFF_H
#define THREADING_STUFF_H

#include <pthread.h>

typedef struct _ThreadConditionPack ThreadConditionPack;

struct _ThreadConditionPack {
  pthread_cond_t *cond_var;
  pthread_mutex_t *mutex_to_use; 
  int cond_to_verify;
};

ThreadConditionPack *init_thread_cond_pack(pthread_cond_t *var, 
                            pthread_mutex_t *cond_mu);
void free_thread_cond_pack(ThreadConditionPack*);
void set_cond_to_verify_to_true(ThreadConditionPack*);
void set_cond_to_verify_to_false(ThreadConditionPack*);

#endif
