#ifndef LISTENER_H
#define LISTENER_H

#include <stdio.h>
#include <pthread.h>

#include "network_interactions.h"
#include "threading_stuff.h"

typedef struct _ListenerMutexSet ListenerMutexSet;
typedef struct _ListenerThreadCondPackets ListenerThreadCondPackets;
typedef struct _ListenerPack ListenerPack;
typedef enum _listener_t listener_t;

enum _listener_t {
  LISTENER_FIRST = 1,
  LISTENER_SECOND = 2
};

struct _ListenerPack {
  listener_t type;
  int sd;
  ClientAddress *client_addr;
  ListenerMutexSet *mu_set;
  BatchStock *batch_stock;
  ListenerThreadCondPackets *cond_packs;
};

struct _ListenerMutexSet {
  pthread_mutex_t *batch_stock_access_mu;
  pthread_mutex_t *client_addr_mu;
  pthread_mutex_t *io_mu; 
};

struct _ListenerThreadCondPackets {
  ThreadConditionPack *listener_signal;
  ThreadConditionPack *calc_to_listener_signal;
};

ListenerPack *initialize_listener_pack(listener_t type, int sd, 
                        ClientAddress *client_addr, ListenerMutexSet *mu_set, 
                        ListenerThreadCondPackets *thread_cond_packets, 
                        BatchStock *bs);

ListenerThreadCondPackets *initialize_cond_packs(
                                ThreadConditionPack *listener_signal,
                                ThreadConditionPack *calc_to_listener_signal);

ListenerMutexSet *create_listener_mutex_set(
                                        pthread_mutex_t *batch_stock_access_mu,
                                        pthread_mutex_t *client_addr_mu, 
                                        pthread_mutex_t *io_mu);

void free_listener_pack(ListenerPack *lp);
void free_listener_mutex_set(ListenerMutexSet *mu_set);

void run_listener(ListenerPack *lp);

#endif
