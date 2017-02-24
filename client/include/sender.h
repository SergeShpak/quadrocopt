#ifndef SENDER_H
#define SENDER_H

#include <pthread.h>
#include <stdlib.h>

#include "data_struct.h"
#include "threading_stuff.h"
#include "network_interactions.h"

typedef struct _SenderMutexSet SenderMutexSet;
typedef struct _SenderPack SenderPack;
typedef struct _SenderThreadCondPacks SenderThreadCondPacks;

struct _SenderMutexSet {
  pthread_mutex_t *io_mu;
};

struct _SenderThreadCondPacks {
  ThreadConditionPack *calc_to_sender_signal;
  ThreadConditionPack *sender_signal;
};

struct _SenderPack {
  int sd;
  ServerAddress *server_addr;
  SenderStock *stock; 
  SenderMutexSet *mu_set;
  SenderThreadCondPacks *cond_packs;
};

SenderPack *initialize_sender_pack(int sd, ServerAddress *server_addr, 
                                SenderStock *stock, SenderMutexSet *mu_set, 
                                SenderThreadCondPacks *cond_packs);
void free_sender_pack(SenderPack *sp);

SenderMutexSet *initialize_sender_mutex_set(pthread_mutex_t *io_mu);
void free_sender_mu_set(SenderMutexSet *mu_set);

SenderThreadCondPacks *initialize_sender_cond_packs(
                                  ThreadConditionPack *sender_signal,
                                  ThreadConditionPack *calc_to_sender_signal);
void free_sender_cond_packs(SenderThreadCondPacks *stcp);

void run_sender(SenderPack *sp);
#endif
