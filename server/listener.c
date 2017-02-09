#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "include/listener.h"

void run_listener(ListenerPack *lp) {
  int i;
  while(1) {
    scanf("%d\n", &i);
    pthread_mutex_lock(lp->mu_set->io_mu);
    printf("Listener %d read number %d\n", lp->type, i); 
    pthread_mutex_unlock(lp->mu_set->io_mu);
  }
}

ListenerPack *initialize_listener_pack(listener_t type, int sd, 
                    ListenerMutexSet *mu_set, BatchStock *bs, FILE *log_file) {
  ListenerPack *lp = (ListenerPack *) malloc(sizeof(ListenerPack));
  lp->type = type;
  lp->sd = sd;
  lp->mu_set = mu_set;
  lp->batch_stock = bs;
  lp->log_file = log_file;
  return lp;
}

void free_listener_pack(ListenerPack *lp) {
  free(lp);
}

ListenerMutexSet *create_listener_mutex_set(pthread_mutex_t *batch_stock_mu,
                                            pthread_mutex_t *io_mu) {
  ListenerMutexSet *lms = (ListenerMutexSet *)malloc(sizeof(ListenerMutexSet));
  lms->batch_stock_mu = batch_stock_mu;
  lms->io_mu = io_mu;
  return lms;
}

void free_listener_mutex_set(ListenerMutexSet *mu_set) {
  free(mu_set);
}
