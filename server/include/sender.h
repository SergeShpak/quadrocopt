#ifndef SENDER_H
#define SENDER_H

#include <pthread.h>
#include <stdlib.h>

#include "data_struct.h"
#include "threading_stuff.h"

typedef struct _SenderMutexSet SenderMutexSet;
typedef struct _SenderPack SenderPack;
typedef struct _SenderThreadCondPacks SenderThreadCondPacks;

struct _SenderMutexSet {
  pthread_mutex_t *io_mu;
};

struct _SenderThreadCondPacks {
  ThreadConditionPack *reader_calculated_cond_pack;
  ThreadConditionPack *sender_sent_cond_pack;
};

struct _SenderPack {
  int sd;
  CalculationsStock *cs; 
  SenderMutexSet *mu_set;
  SenderThreadCondPacks *cond_packs;
};

SenderPack *initialize_sender_pack(int sd, CalculationsStock *cs, 
                    SenderMutexSet *mu_set, SenderThreadCondPacks *cond_packs);
void free_sender_pack(SenderPack *sp);

SenderMutexSet *initialize_sender_mutex_set(pthread_mutex_t *io_mu);
void free_sender_mu_set(SenderMutexSet *mu_set);

SenderThreadCondPacks *initialize_sender_cond_packs(
                      ThreadConditionPack *reader_calculated_cond_pack,
                      ThreadConditionPack *sender_sent_cond_pack);
void free_sender_cond_packs(SenderThreadCondPacks *stcp);

void run_sender(SenderPack *sp);
#endif
