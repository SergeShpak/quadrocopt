#ifndef LISTENER_H
#define LISTENER_H

#include <stdio.h>
#include <pthread.h>

#include "network_interactions.h"

typedef struct _ListenerMutexSet ListenerMutexSet;
typedef struct _ListenerPack ListenerPack;
typedef enum _listener_t listener_t;

enum _listener_t {
  LISTENER_FIRST = 1,
  LISTENER_SECOND = 2
};

struct _ListenerPack {
  listener_t type;
  int sd;
  FILE *log_file;
  ListenerMutexSet *mu_set;
  BatchStock *batch_stock;
};

struct _ListenerMutexSet {
  pthread_mutex_t *batch_stock_mu;
  pthread_mutex_t *io_mu; 
};

ListenerPack *initialize_listener_pack(listener_t type, int sd, 
                    ListenerMutexSet *mu_set, BatchStock *bs, FILE *log_file);

ListenerMutexSet *create_listener_mutex_set(pthread_mutex_t *batch_stock_mu,
                                            pthread_mutex_t *io_mu);

void free_listener_pack(ListenerPack *lp);
void free_listener_mutex_set(ListenerMutexSet *mu_set);

void run_listener(ListenerPack *lp);

#endif
